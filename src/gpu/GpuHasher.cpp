#include "GpuHasher.hpp"
#include "../utils/Logger.hpp"
#include "../core/Constants.hpp"

#include <algorithm>
#include <cstring>

namespace fenrir {
namespace gpu {

GpuHasher::GpuHasher(std::shared_ptr<OpenCLContext> context)
    : m_context(std::move(context))
    , m_kernelCache(m_context.get() ? m_context->context() : nullptr,
                    m_context.get() ? m_context->deviceId() : nullptr,
                    m_context.get() ? m_context->commandQueue() : nullptr)
{
}

bool GpuHasher::initialize(const std::string& hashType,
                            const std::string& kernelPath,
                            const std::string& kernelFuncName) {
    if (!m_context || !m_context->isAvailable()) {
        utils::Logger::warn("OpenCL not available — using CPU fallback");
        return false;
    }

    m_currentKernel = m_kernelCache.getOrBuild(hashType, kernelPath, kernelFuncName);
    if (!m_currentKernel || !m_currentKernel->isReady()) {
        utils::Logger::error("Failed to initialize GPU kernel for " + hashType);
        return false;
    }

    autoTuneWorkSize();
    return true;
}

void GpuHasher::hashBatch(const std::vector<std::string>& candidates,
                           const std::vector<uint32_t>& targets,
                           std::vector<size_t>& found) {
    found.clear();
    if (!isReady() || candidates.empty()) return;

#ifdef FENRIR_HAS_GPU
    std::vector<uint8_t>  packed;
    std::vector<uint32_t> offsets;
    std::vector<uint8_t>  lengths;
    packCandidates(candidates, packed, offsets, lengths);

    cl_int err;
    cl_context ctx = m_context->context();
    cl_command_queue queue = m_context->commandQueue();

    struct BufferGuard {
        std::vector<cl_mem> bufs;
        ~BufferGuard() {
            for (auto b : bufs) if (b) clReleaseMemObject(b);
        }
        void add(cl_mem b) { if (b) bufs.push_back(b); }
    } guard;

    cl_mem d_packed = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                     packed.size(), packed.data(), &err);
    if (err != CL_SUCCESS) return;
    guard.add(d_packed);

    cl_mem d_offsets = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                      offsets.size() * sizeof(uint32_t), offsets.data(), &err);
    if (err != CL_SUCCESS) return;
    guard.add(d_offsets);

    cl_mem d_lengths = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                      lengths.size() * sizeof(uint8_t), lengths.data(), &err);
    if (err != CL_SUCCESS) return;
    guard.add(d_lengths);

    // Results buffer: kernel writes 4 uint32 per candidate (16 bytes each)
    size_t resultSize = candidates.size() * 4 * sizeof(uint32_t);
    cl_mem d_results = clCreateBuffer(ctx, CL_MEM_READ_WRITE, resultSize, nullptr, &err);
    if (err != CL_SUCCESS) return;
    guard.add(d_results);

    // Target prefix: copy first target's first word as the kernel target
    // For now, use the first target's prefix since kernel only handles single target
    uint32_t targetPrefix = targets.empty() ? 0xFFFFFFFF : targets[0];
    // Use a 4-byte buffer for the constant target_prefix (kernel reads __constant uint*)
    cl_mem d_targetPrefix = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                           sizeof(uint32_t), &targetPrefix, &err);
    if (err != CL_SUCCESS) { clReleaseMemObject(d_results); return; }
    guard.add(d_targetPrefix);

    // Found flag: single int initialized to large value for atom_min to work
    int32_t foundInit = static_cast<int32_t>(candidates.size() + 1);
    cl_mem d_found = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                    sizeof(int32_t), &foundInit, &err);
    if (err != CL_SUCCESS) { clReleaseMemObject(d_targetPrefix); return; }
    guard.add(d_found);

    m_currentKernel->setArgBuffer(0, sizeof(cl_mem), &d_packed);
    m_currentKernel->setArgBuffer(1, sizeof(cl_mem), &d_offsets);
    m_currentKernel->setArgBuffer(2, sizeof(cl_mem), &d_lengths);
    m_currentKernel->setArgBuffer(3, sizeof(cl_mem), &d_results);
    m_currentKernel->setArgBuffer(4, sizeof(cl_mem), &d_targetPrefix);
    m_currentKernel->setArgBuffer(5, sizeof(cl_mem), &d_found);

    size_t globalWorkSize = candidates.size();
    size_t remainder = globalWorkSize % m_localWorkSize;
    if (remainder != 0) {
        globalWorkSize += (m_localWorkSize - remainder);
    }

    m_currentKernel->execute(globalWorkSize, m_localWorkSize);

    // Read found flag
    int32_t foundIdx = 0;
    err = clEnqueueReadBuffer(queue, d_found, CL_TRUE, 0,
                              sizeof(int32_t), &foundIdx, 0, nullptr, nullptr);

    if (err == CL_SUCCESS && foundIdx >= 0 &&
        static_cast<size_t>(foundIdx) < candidates.size()) {
        found.push_back(static_cast<size_t>(foundIdx));
    }
#endif
}

bool GpuHasher::isReady() const {
    return m_context && m_context->isAvailable() && m_currentKernel;
}

size_t GpuHasher::localWorkSize() const {
    return m_localWorkSize;
}

void GpuHasher::packCandidates(const std::vector<std::string>& candidates,
                                std::vector<uint8_t>& packed,
                                std::vector<uint32_t>& offsets,
                                std::vector<uint8_t>& lengths) {

    size_t totalBytes = 0;
    for (const auto& c : candidates) {
        totalBytes += c.size();
    }

    packed.resize(totalBytes);
    offsets.resize(candidates.size());
    lengths.resize(candidates.size());

    size_t pos = 0;
    for (size_t i = 0; i < candidates.size(); ++i) {
        offsets[i] = static_cast<uint32_t>(pos);
        lengths[i] = static_cast<uint8_t>(candidates[i].size());
        std::memcpy(packed.data() + pos, candidates[i].data(), candidates[i].size());
        pos += candidates[i].size();
    }
}

void GpuHasher::autoTuneWorkSize() {
    if (!m_context) {
        m_localWorkSize = 256;
        return;
    }

    size_t maxWG = m_context->maxWorkGroupSize();

    m_localWorkSize = std::min(size_t(256), maxWG);
    if (m_localWorkSize < 64) m_localWorkSize = 64;


}

}
}
