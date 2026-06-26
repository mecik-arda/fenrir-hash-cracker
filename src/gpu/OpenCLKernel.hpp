#pragma once

#include <string>
#include <cstddef>

#ifdef FENRIR_HAS_GPU
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#endif

namespace fenrir {
namespace gpu {

#ifdef FENRIR_HAS_GPU
using OclContext = cl_context;
using OclDevice  = cl_device_id;
using OclQueue   = cl_command_queue;
using OclKernel  = cl_kernel;
using OclProgram = cl_program;
#else
using OclContext = void*;
using OclDevice  = void*;
using OclQueue   = void*;
using OclKernel  = void*;
using OclProgram = void*;
#endif

class OpenCLKernel {
public:
    OpenCLKernel(OclContext context, OclDevice device, OclQueue queue);
    ~OpenCLKernel();


    bool build(const std::string& sourcePath, const std::string& functionName);


    bool buildFromSource(const std::string& source,
                         const std::string& functionName);


    template<typename T>
    bool setArg(int index, T* buffer) {
#ifdef FENRIR_HAS_GPU
        if (!m_kernel) return false;
        return clSetKernelArg(m_kernel, index, sizeof(T), buffer) == CL_SUCCESS;
#else
        return false;
#endif
    }


    bool setArgBuffer(int index, size_t size, void* buffer);


    bool execute(size_t globalWorkSize, size_t localWorkSize);


    std::string buildLog() const;

    OclKernel kernel() const {
#ifdef FENRIR_HAS_GPU
        return m_kernel;
#else
        return nullptr;
#endif
    }
    bool isReady() const;

private:
#ifdef FENRIR_HAS_GPU
    OclContext m_context;
    OclDevice  m_device;
    OclQueue   m_queue;
    OclKernel  m_kernel = nullptr;
    OclProgram m_program = nullptr;
#endif
};

}
}
