#include "OpenCLContext.hpp"
#include "../utils/Logger.hpp"

namespace fenrir {
namespace gpu {

OpenCLContext::OpenCLContext(int deviceIndex) {
#ifdef FENRIR_HAS_GPU
    cl_int err;


    cl_uint numPlatforms = 0;
    err = clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (err != CL_SUCCESS || numPlatforms == 0) {
        utils::Logger::warn("No OpenCL platforms found — GPU disabled");
        return;
    }

    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);


    m_platform = platforms[0];


    cl_uint numDevices = 0;
    err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    if (err != CL_SUCCESS || numDevices == 0) {

        utils::Logger::warn("No GPU devices found — trying CPU device");
        err = clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices);
        if (err != CL_SUCCESS || numDevices == 0) {
            utils::Logger::warn("No OpenCL devices available");
            return;
        }
    }

    std::vector<cl_device_id> devices(numDevices);
    clGetDeviceIDs(m_platform, CL_DEVICE_TYPE_ALL, numDevices, devices.data(), nullptr);

    if (deviceIndex >= static_cast<int>(numDevices)) {
        utils::Logger::warn("Device index " + std::to_string(deviceIndex) +
                           " out of range (" + std::to_string(numDevices) + " devices)");
        deviceIndex = 0;
    }
    m_device = devices[deviceIndex];


    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
        utils::Logger::error("Failed to create OpenCL context");
        return;
    }


    m_queue = clCreateCommandQueue(m_context, m_device, 0, &err);
    if (err != CL_SUCCESS) {
        utils::Logger::error("Failed to create OpenCL command queue");
        clReleaseContext(m_context);
        m_context = nullptr;
        return;
    }

    m_available = true;

    char nameBuf[256] = {};
    clGetDeviceInfo(m_device, CL_DEVICE_NAME, sizeof(nameBuf), nameBuf, nullptr);
    utils::Logger::info("OpenCL device: " + std::string(nameBuf));
#else
    utils::Logger::info("OpenCL support not compiled in — CPU only");
#endif
}

OpenCLContext::~OpenCLContext() {
#ifdef FENRIR_HAS_GPU
    if (m_queue)    clReleaseCommandQueue(m_queue);
    if (m_context)  clReleaseContext(m_context);
#endif
}

bool OpenCLContext::isAvailable() const {
#ifdef FENRIR_HAS_GPU
    return m_available;
#else
    return false;
#endif
}

std::string OpenCLContext::deviceName() const {
#ifdef FENRIR_HAS_GPU
    if (!m_device) return "None";
    char buf[256] = {};
    clGetDeviceInfo(m_device, CL_DEVICE_NAME, sizeof(buf), buf, nullptr);
    return std::string(buf);
#else
    return "CPU (no OpenCL)";
#endif
}

int OpenCLContext::computeUnits() const {
#ifdef FENRIR_HAS_GPU
    if (!m_device) return 0;
    cl_uint cu = 0;
    clGetDeviceInfo(m_device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cu), &cu, nullptr);
    return static_cast<int>(cu);
#else
    return 0;
#endif
}

size_t OpenCLContext::maxWorkGroupSize() const {
#ifdef FENRIR_HAS_GPU
    if (!m_device) return 0;
    size_t s = 0;
    clGetDeviceInfo(m_device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(s), &s, nullptr);
    return s;
#else
    return 0;
#endif
}

size_t OpenCLContext::localMemorySize() const {
#ifdef FENRIR_HAS_GPU
    if (!m_device) return 0;
    cl_ulong s = 0;
    clGetDeviceInfo(m_device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(s), &s, nullptr);
    return static_cast<size_t>(s);
#else
    return 0;
#endif
}

std::string OpenCLContext::deviceVendor() const {
#ifdef FENRIR_HAS_GPU
    if (!m_device) return "None";
    char buf[256] = {};
    clGetDeviceInfo(m_device, CL_DEVICE_VENDOR, sizeof(buf), buf, nullptr);
    return std::string(buf);
#else
    return "None";
#endif
}

}
}
