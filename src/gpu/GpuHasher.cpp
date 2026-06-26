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

    cl_mem d_targets = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                      targets.size() * sizeof(uint32_t), (void*)targets.data(), &err);
    if (err != CL_SUCCESS) return;
    guard.add(d_targets);

    std::vector<uint32_t> foundFlags(candidates.size(), 0);
    cl_mem d_found = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                    foundFlags.size() * sizeof(uint32_t), foundFlags.data(), &err);
    if (err != CL_SUCCESS) return;
    guard.add(d_found);

    uint32_t numTargets = static_cast<uint32_t>(targets.size());

    m_currentKernel->setArgBuffer(0, sizeof(cl_mem), &d_packed);
    m_currentKernel->setArgBuffer(1, sizeof(cl_mem), &d_offsets);
    m_currentKernel->setArgBuffer(2, sizeof(cl_mem), &d_lengths);
    m_currentKernel->setArgBuffer(3, sizeof(cl_mem), &d_targets);
    m_currentKernel->setArgBuffer(4, sizeof(uint32_t), &numTargets);
    m_currentKernel->setArgBuffer(5, sizeof(cl_mem), &d_found);

    size_t globalWorkSize = candidates.size();
    
    size_t remainder = globalWorkSize % m_localWorkSize;
    if (remainder != 0) {
        globalWorkSize += (m_localWorkSize - remainder);
    }

    m_currentKernel->execute(globalWorkSize, m_localWorkSize);

    err = clEnqueueReadBuffer(queue, d_found, CL_TRUE, 0,
                              foundFlags.size() * sizeof(uint32_t), foundFlags.data(),
                              0, nullptr, nullptr);

    if (err == CL_SUCCESS) {
        for (size_t i = 0; i < candidates.size(); ++i) {
            if (foundFlags[i] != 0) {
                found.push_back(i);
            }
        }
    } else {
        utils::Logger::error("Failed to read result buffer from GPU");
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
