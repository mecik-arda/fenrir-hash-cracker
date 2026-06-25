
find_path(OpenCL_INCLUDE_DIR
    NAMES CL/cl.h OpenCL/cl.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/rocm/include
        /opt/amdgpu-pro/include
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v*/include"
        ${CUDA_INC_DIR}
        ENV OpenCL_INC_DIR
        ENV AMDAPPSDKROOT/include
        ENV INTELOCLSDKROOT/include
        ENV NVSDKCOMPUTE_ROOT/include
)

find_library(OpenCL_LIBRARY
    NAMES OpenCL libOpenCL
    PATHS
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
        /opt/rocm/lib
        /opt/amdgpu-pro/lib
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v*/lib/x64"
        ENV OpenCL_LIB_DIR
        ENV AMDAPPSDKROOT/lib/x86_64
        ENV INTELOCLSDKROOT/lib/x64
        ENV NVSDKCOMPUTE_ROOT/lib/x64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenCL
    REQUIRED_VARS OpenCL_LIBRARY OpenCL_INCLUDE_DIR
)

if(OpenCL_FOUND AND NOT TARGET OpenCL::OpenCL)
    add_library(OpenCL::OpenCL UNKNOWN IMPORTED)
    set_target_properties(OpenCL::OpenCL PROPERTIES
        IMPORTED_LOCATION "${OpenCL_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenCL_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(OpenCL_INCLUDE_DIR OpenCL_LIBRARY)
