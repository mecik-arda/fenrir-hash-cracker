#include "AsyncGpuHasher.hpp"
#include "../utils/Logger.hpp"
#include "../core/Constants.hpp"
#include <cstring>
#include <algorithm>

namespace fenrir { namespace gpu {

GPUBufferSet::~GPUBufferSet() { release(); }

void GPUBufferSet::release() {
#ifdef FENRIR_HAS_GPU
    if (d_candidates) { clReleaseMemObject(d_candidates); d_candidates = nullptr; }
    if (d_offsets)    { clReleaseMemObject(d_offsets);    d_offsets    = nullptr; }
    if (d_lengths)    { clReleaseMemObject(d_lengths);    d_lengths    = nullptr; }
    if (d_results)    { clReleaseMemObject(d_results);    d_results    = nullptr; }
    if (d_found)      { clReleaseMemObject(d_found);      d_found      = nullptr; }
    if (writeEvent)   { clReleaseEvent(writeEvent);       writeEvent   = nullptr; }
    if (kernelEvent)  { clReleaseEvent(kernelEvent);      kernelEvent  = nullptr; }
#endif
}

void GPUBufferSet::allocate(size_t maxCandidates, size_t maxBytes) {
#ifdef FENRIR_HAS_GPU
    release();
    cl_context ctx = nullptr;
    cl_int err;
    hostPacked.resize(maxBytes, 0);
    hostOffsets.resize(maxCandidates, 0);
    hostLengths.resize(maxCandidates, 0);
    hostResults.resize(maxCandidates * 8, 0);
    d_candidates = clCreateBuffer(ctx, CL_MEM_READ_ONLY, maxBytes, nullptr, &err);
    d_offsets    = clCreateBuffer(ctx, CL_MEM_READ_ONLY, maxCandidates * sizeof(uint32_t), nullptr, &err);
    d_lengths    = clCreateBuffer(ctx, CL_MEM_READ_ONLY, maxCandidates, nullptr, &err);
    d_results    = clCreateBuffer(ctx, CL_MEM_READ_WRITE, maxCandidates * 8 * sizeof(uint32_t), nullptr, &err);
    d_found      = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof(int), nullptr, &err);
    candidateCount = 0;
#endif
}

AsyncGpuHasher::AsyncGpuHasher(std::shared_ptr<OpenCLContext> context)
    : m_context(std::move(context))
    , m_kernelCache(m_context ? m_context->context() : nullptr,
                    m_context ? m_context->deviceId() : nullptr,
                    m_context ? m_context->commandQueue() : nullptr) {}

AsyncGpuHasher::~AsyncGpuHasher() { m_buffers.clear(); }

bool AsyncGpuHasher::initialize(const std::string& hashType, const std::string& kernelPath,
                                 const std::string& kernelFuncName, int numBuffers) {
    if (!m_context || !m_context->isAvailable()) return false;
    m_numBuffers = numBuffers;
    m_currentKernel = m_kernelCache.getOrBuild(hashType, kernelPath, kernelFuncName);
    if (!m_currentKernel || !m_currentKernel->isReady()) return false;
    autoTuneWorkSize();
    m_maxCandidates = core::DEFAULT_FAST_HASH_BATCH_SIZE;
    m_buffers.resize(m_numBuffers);
    for (int i = 0; i < m_numBuffers; i++) m_buffers[i] = std::make_unique<GPUBufferSet>();
    m_currentWrite = 0;
    m_currentCompute = 0;
    m_pendingSubmits = 0;
    return true;
}

bool AsyncGpuHasher::submitBatch(const std::vector<std::string>& candidates,
                                  const std::vector<uint32_t>& targetPrefixes,
                                  std::vector<size_t>& foundIndices) {
    foundIndices.clear();
    if (!isReady() || candidates.empty()) return false;

    if (m_pendingSubmits >= m_numBuffers) {
        if (!waitForCompletion(foundIndices)) return false;
    }

    int bufIdx = m_currentWrite % m_numBuffers;
    auto& buf = m_buffers[bufIdx];

    packCandidates(candidates, buf->hostPacked, buf->hostOffsets, buf->hostLengths);
    buf->candidateCount = candidates.size();
    m_lastTargetPrefixes = targetPrefixes;

#ifdef FENRIR_HAS_GPU
    cl_int err;
    cl_command_queue q = m_context->commandQueue();

    clEnqueueWriteBuffer(q, buf->d_candidates, CL_FALSE, 0, buf->hostPacked.size(),
                         buf->hostPacked.data(), 0, nullptr, &buf->writeEvent);
    clEnqueueWriteBuffer(q, buf->d_offsets, CL_FALSE, 0,
                         candidates.size() * sizeof(uint32_t),
                         buf->hostOffsets.data(), 0, nullptr, nullptr);
    clEnqueueWriteBuffer(q, buf->d_lengths, CL_FALSE, 0, candidates.size(),
                         buf->hostLengths.data(), 0, nullptr, nullptr);

    int foundInit = -1;
    clEnqueueWriteBuffer(q, buf->d_found, CL_FALSE, 0, sizeof(int),
                         &foundInit, 0, nullptr, nullptr);
    clFlush(q);

    m_currentWrite++;
    m_pendingSubmits++;
#endif
    return true;
}

bool AsyncGpuHasher::waitForCompletion(std::vector<size_t>& foundIndices) {
    foundIndices.clear();
    if (m_pendingSubmits <= 0) return true;

    int bufIdx = m_currentCompute % m_numBuffers;
    auto& buf = m_buffers[bufIdx];

#ifdef FENRIR_HAS_GPU
    cl_int err;
    cl_command_queue q = m_context->commandQueue();

    cl_event kernelDone;
    size_t globalSize = ((buf->candidateCount + m_localWorkSize - 1) / m_localWorkSize) * m_localWorkSize;

    clSetKernelArg(m_currentKernel->kernel(), 0, sizeof(cl_mem), &buf->d_candidates);
    clSetKernelArg(m_currentKernel->kernel(), 1, sizeof(cl_mem), &buf->d_offsets);
    clSetKernelArg(m_currentKernel->kernel(), 2, sizeof(cl_mem), &buf->d_lengths);
    clSetKernelArg(m_currentKernel->kernel(), 3, sizeof(cl_mem), &buf->d_results);
    clSetKernelArg(m_currentKernel->kernel(), 4, sizeof(cl_mem), &buf->d_found);

    clEnqueueNDRangeKernel(q, m_currentKernel->kernel(), 1, nullptr,
                           &globalSize, &m_localWorkSize,
                           0, nullptr, &kernelDone);

    clFlush(q);
    clWaitForEvents(1, &kernelDone);
    clReleaseEvent(kernelDone);

    clEnqueueReadBuffer(q, buf->d_found, CL_TRUE, 0, sizeof(int),
                        &buf->hostFound, 0, nullptr, nullptr);

    if (buf->hostFound >= 0 && static_cast<size_t>(buf->hostFound) < buf->candidateCount) {
        clEnqueueReadBuffer(q, buf->d_results, CL_TRUE, 0,
                            buf->candidateCount * 8 * sizeof(uint32_t),
                            buf->hostResults.data(), 0, nullptr, nullptr);
        foundIndices.push_back(static_cast<size_t>(buf->hostFound));
    }
#endif

    m_currentCompute++;
    m_pendingSubmits--;
    return true;
}

bool AsyncGpuHasher::isReady() const {
    return m_context && m_context->isAvailable() && m_currentKernel != nullptr;
}

size_t AsyncGpuHasher::localWorkSize() const { return m_localWorkSize; }

void AsyncGpuHasher::packCandidates(const std::vector<std::string>& candidates,
                                     std::vector<uint8_t>& packed, std::vector<uint32_t>& offsets,
                                     std::vector<uint8_t>& lengths) {
    size_t total = 0;
    for (auto& c : candidates) total += c.size();
    packed.resize(total);
    offsets.resize(candidates.size());
    lengths.resize(candidates.size());
    size_t pos = 0;
    for (size_t i = 0; i < candidates.size(); i++) {
        offsets[i] = static_cast<uint32_t>(pos);
        lengths[i] = static_cast<uint8_t>(candidates[i].size());
        memcpy(packed.data() + pos, candidates[i].data(), candidates[i].size());
        pos += candidates[i].size();
    }
}

void AsyncGpuHasher::autoTuneWorkSize() {
    if (!m_context) { m_localWorkSize = 256; return; }
    size_t maxWG = m_context->maxWorkGroupSize();
    m_localWorkSize = std::min(size_t(256), maxWG);
    if (m_localWorkSize < 64) m_localWorkSize = 64;
}

} }
