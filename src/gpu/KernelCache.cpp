#include "KernelCache.hpp"

namespace fenrir {
namespace gpu {

KernelCache::KernelCache(OclContext context, OclDevice device, OclQueue queue)
    : m_context(context), m_device(device), m_queue(queue) {}

OpenCLKernel* KernelCache::getOrBuild(const std::string& hashType,
                                       const std::string& kernelPath,
                                       const std::string& functionName) {

    return nullptr;
}

void KernelCache::clear() {

}

}
}
