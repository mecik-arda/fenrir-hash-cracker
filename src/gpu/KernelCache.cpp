#include "KernelCache.hpp"
#include "../utils/Logger.hpp"

namespace fenrir {
namespace gpu {

KernelCache::KernelCache(OclContext context, OclDevice device, OclQueue queue)
    : m_context(context), m_device(device), m_queue(queue) {}

OpenCLKernel* KernelCache::getOrBuild(const std::string& hashType,
                                       const std::string& kernelPath,
                                       const std::string& functionName) {
    if (!m_context || !m_device || !m_queue) {
        utils::Logger::warn("KernelCache: OpenCL not available");
        return nullptr;
    }

    std::string cacheKey = hashType + ":" + kernelPath + ":" + functionName;

    auto it = m_cache.find(cacheKey);
    if (it != m_cache.end() && it->second && it->second->isReady()) {
        return it->second.get();
    }

    auto kernel = std::make_unique<OpenCLKernel>(m_context, m_device, m_queue);
    if (!kernel->build(kernelPath, functionName)) {
        utils::Logger::error("KernelCache: Failed to build kernel " + functionName +
                            " from " + kernelPath);
        utils::Logger::error("Build log: " + kernel->buildLog());
        return nullptr;
    }

    OpenCLKernel* raw = kernel.get();
    m_cache[cacheKey] = std::move(kernel);
    utils::Logger::debug("KernelCache: Built and cached kernel " + functionName);
    return raw;
}

void KernelCache::clear() {
    m_cache.clear();
}

}
}
