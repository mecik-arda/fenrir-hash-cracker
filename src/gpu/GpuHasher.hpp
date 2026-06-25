#pragma once

#include "OpenCLContext.hpp"
#include "OpenCLKernel.hpp"
#include "KernelCache.hpp"
#include "../core/IHashEngine.hpp"
#include <vector>
#include <string>
#include <memory>

namespace fenrir {
namespace gpu {

class GpuHasher {
public:
    GpuHasher(std::shared_ptr<OpenCLContext> context);


    bool initialize(const std::string& hashType, const std::string& kernelPath,
                    const std::string& kernelFuncName);





    void hashBatch(const std::vector<std::string>& candidates,
                   const std::vector<uint32_t>& targets,
                   std::vector<size_t>& found);


    bool isReady() const;


    size_t localWorkSize() const;

private:
    std::shared_ptr<OpenCLContext> m_context;
    KernelCache m_kernelCache;
    OpenCLKernel* m_currentKernel = nullptr;


    size_t m_localWorkSize = 256;


    void packCandidates(const std::vector<std::string>& candidates,
                        std::vector<uint8_t>& packed,
                        std::vector<uint32_t>& offsets,
                        std::vector<uint8_t>& lengths);


    void autoTuneWorkSize();
};

}
}
