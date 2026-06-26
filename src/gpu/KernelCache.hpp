#pragma once

#include "OpenCLKernel.hpp"
#include <string>
#include <memory>
#include <unordered_map>

namespace fenrir {
namespace gpu {

class KernelCache {
public:
    KernelCache(OclContext context, OclDevice device, OclQueue queue);

    OpenCLKernel* getOrBuild(const std::string& hashType,
                              const std::string& kernelPath,
                              const std::string& functionName);

    void clear();

private:
    OclContext m_context;
    OclDevice  m_device;
    OclQueue   m_queue;
    std::unordered_map<std::string, std::unique_ptr<OpenCLKernel>> m_cache;
};

}
}
