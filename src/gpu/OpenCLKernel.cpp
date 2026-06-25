#include "OpenCLKernel.hpp"
#include "../utils/Logger.hpp"

#include <fstream>
#include <sstream>

namespace fenrir {
namespace gpu {

OpenCLKernel::OpenCLKernel(OclContext context, OclDevice device, OclQueue queue) {
#ifdef FENRIR_HAS_GPU
    m_context = context;
    m_device  = device;
    m_queue   = queue;
#endif
}

OpenCLKernel::~OpenCLKernel() {
#ifdef FENRIR_HAS_GPU
    if (m_kernel)  clReleaseKernel(m_kernel);
    if (m_program) clReleaseProgram(m_program);
#endif
}

bool OpenCLKernel::build(const std::string& sourcePath,
                          const std::string& functionName) {
    std::ifstream file(sourcePath);
    if (!file.is_open()) {
        utils::Logger::error("Cannot open kernel source: " + sourcePath);
        return false;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return buildFromSource(ss.str(), functionName);
}

bool OpenCLKernel::buildFromSource(const std::string& source,
                                    const std::string& functionName) {
#ifdef FENRIR_HAS_GPU
    if (!m_context) return false;

    cl_int err;
    const char* src = source.c_str();
    size_t len = source.size();

    m_program = clCreateProgramWithSource(m_context, 1, &src, &len, &err);
    if (err != CL_SUCCESS) {
        utils::Logger::error("Failed to create OpenCL program");
        return false;
    }

    err = clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);

    if (err != CL_SUCCESS) {
        size_t logLen = 0;
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                              0, nullptr, &logLen);
        std::string log(logLen, ' ');
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                              logLen, log.data(), nullptr);
        utils::Logger::error("Kernel build failed:\n" + log);
        return false;
    }

    m_kernel = clCreateKernel(m_program, functionName.c_str(), &err);
    if (err != CL_SUCCESS) {
        utils::Logger::error("Failed to create kernel: " + functionName);
        return false;
    }

    return true;
#else
    utils::Logger::warn("OpenCL not available — kernel build skipped");
    return false;
#endif
}

bool OpenCLKernel::setArgBuffer(int index, size_t size, void* buffer) {
#ifdef FENRIR_HAS_GPU
    if (!m_kernel) return false;
    return clSetKernelArg(m_kernel, index, size, buffer) == CL_SUCCESS;
#else
    return false;
#endif
}

bool OpenCLKernel::execute(size_t globalWorkSize, size_t localWorkSize) {
#ifdef FENRIR_HAS_GPU
    if (!m_kernel || !m_queue) return false;
    return clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr,
                                   &globalWorkSize, &localWorkSize,
                                   0, nullptr, nullptr) == CL_SUCCESS;
#else
    return false;
#endif
}

std::string OpenCLKernel::buildLog() const {
#ifdef FENRIR_HAS_GPU
    if (!m_program) return "No program";
    size_t logLen = 0;
    clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                          0, nullptr, &logLen);
    std::string log(logLen, ' ');
    clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                          logLen, log.data(), nullptr);
    return log;
#else
    return "OpenCL not available";
#endif
}

bool OpenCLKernel::isReady() const {
#ifdef FENRIR_HAS_GPU
    return m_kernel != nullptr;
#else
    return false;
#endif
}

}
}
