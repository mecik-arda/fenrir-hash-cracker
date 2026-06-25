#pragma once

#include <string>
#include <vector>
#include <memory>

#ifdef FENRIR_HAS_GPU
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#endif

namespace fenrir {
namespace gpu {

class OpenCLContext {
public:
    OpenCLContext(int deviceIndex = 0);
    ~OpenCLContext();


    bool isAvailable() const;


    std::string deviceName() const;


    int computeUnits() const;


    size_t maxWorkGroupSize() const;


    size_t localMemorySize() const;


    std::string deviceVendor() const;

#ifdef FENRIR_HAS_GPU
    cl_context       context()       const { return m_context; }
    cl_command_queue commandQueue()  const { return m_queue; }
    cl_device_id     deviceId()      const { return m_device; }
    cl_platform_id   platformId()    const { return m_platform; }
#endif

private:
    bool m_available = false;

#ifdef FENRIR_HAS_GPU
    cl_platform_id   m_platform   = nullptr;
    cl_device_id     m_device     = nullptr;
    cl_context       m_context    = nullptr;
    cl_command_queue m_queue = nullptr;
#endif
};

}
}
