#pragma once
#include "OpenCLContext.hpp"
#include "OpenCLKernel.hpp"
#include "KernelCache.hpp"
#include "../core/IHashEngine.hpp"
#include <vector>
#include <string>
#include <memory>
#include <atomic>

namespace fenrir { namespace gpu {

struct GPUBufferSet {
#ifdef FENRIR_HAS_GPU
    cl_mem d_candidates = nullptr;
    cl_mem d_offsets    = nullptr;
    cl_mem d_lengths    = nullptr;
    cl_mem d_results    = nullptr;
    cl_mem d_found      = nullptr;
    cl_event writeEvent = nullptr;
    cl_event kernelEvent = nullptr;
#endif
    std::vector<uint8_t>  hostPacked;
    std::vector<uint32_t> hostOffsets;
    std::vector<uint8_t>  hostLengths;
    std::vector<uint32_t> hostResults;
    int hostFound = -1;
    size_t candidateCount = 0;

    ~GPUBufferSet();
    void allocate(size_t maxCandidates, size_t maxBytes);
    void release();
};

class AsyncGpuHasher {
public:
    explicit AsyncGpuHasher(std::shared_ptr<OpenCLContext> context);
    ~AsyncGpuHasher();

    bool initialize(const std::string& hashType, const std::string& kernelPath,
                    const std::string& kernelFuncName, int numBuffers = 2);

    bool submitBatch(const std::vector<std::string>& candidates,
                     const std::vector<uint32_t>& targetPrefixes,
                     std::vector<size_t>& foundIndices);

    bool waitForCompletion(std::vector<size_t>& foundIndices);

    bool isReady() const;
    size_t localWorkSize() const;
    int bufferCount() const { return m_numBuffers; }

private:
    std::shared_ptr<OpenCLContext> m_context;
    KernelCache m_kernelCache;
    OpenCLKernel* m_currentKernel = nullptr;
    int m_numBuffers = 2;
    int m_currentWrite = 0;
    int m_currentCompute = 0;
    std::atomic<int> m_pendingSubmits{0};
    size_t m_localWorkSize = 256;
    size_t m_maxCandidates = 0;
    std::vector<std::unique_ptr<GPUBufferSet>> m_buffers;
    std::vector<uint32_t> m_lastTargetPrefixes;

    void packCandidates(const std::vector<std::string>& candidates,
                        std::vector<uint8_t>& packed, std::vector<uint32_t>& offsets,
                        std::vector<uint8_t>& lengths);
    void autoTuneWorkSize();
};

} }
