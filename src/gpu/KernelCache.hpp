#pragma once

#include "OpenCLKernel.hpp"
#include <string>
#include <memory>

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

};

}
}
