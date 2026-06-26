cmake_minimum_required(VERSION 3.20)
project(fenrir-hash-cracker
    VERSION 1.1.2
    DESCRIPTION "GPU-accelerated password hash auditing tool"
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FENRIR_USE_OPENCL  "Enable GPU acceleration via OpenCL"   ON)
option(FENRIR_USE_CURL    "Enable online API attack mode"         ON)
option(FENRIR_BUILD_TESTS "Build unit tests"                     OFF)
option(FENRIR_ENABLE_ASAN "Enable AddressSanitizer (debug only)" OFF)

if(FENRIR_ENABLE_ASAN)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    endif()
endif()

include(FetchContent)

FetchContent_Declare(cli11
    GIT_REPOSITORY  https://github.com/CLIUtils/CLI11.git
    GIT_TAG         v2.5.0)
FetchContent_MakeAvailable(cli11)

FetchContent_Declare(json
    GIT_REPOSITORY  https://github.com/nlohmann/json.git
    GIT_TAG         v3.11.3)
FetchContent_MakeAvailable(json)

FetchContent_Declare(spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog.git
    GIT_TAG         v1.15.3)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(argon2
    GIT_REPOSITORY  https://github.com/P-H-C/phc-winner-argon2.git
    GIT_TAG         master)
FetchContent_Populate(argon2)
add_library(fenrir_argon2 STATIC
    ${argon2_SOURCE_DIR}/src/argon2.c
    ${argon2_SOURCE_DIR}/src/core.c
    ${argon2_SOURCE_DIR}/src/blake2/blake2b.c
    ${argon2_SOURCE_DIR}/src/thread.c
    ${argon2_SOURCE_DIR}/src/encoding.c
    ${argon2_SOURCE_DIR}/src/opt.c
)
target_include_directories(fenrir_argon2 PUBLIC ${argon2_SOURCE_DIR}/include)

FetchContent_Declare(libbcrypt
    GIT_REPOSITORY  https://github.com/trusch/libbcrypt.git
    GIT_TAG         master)
FetchContent_Populate(libbcrypt)
add_library(fenrir_bcrypt STATIC
    ${libbcrypt_SOURCE_DIR}/src/bcrypt.c
    ${libbcrypt_SOURCE_DIR}/src/crypt_blowfish.c
    ${libbcrypt_SOURCE_DIR}/src/crypt_gensalt.c
    ${libbcrypt_SOURCE_DIR}/src/wrapper.c
)
target_include_directories(fenrir_bcrypt PUBLIC ${libbcrypt_SOURCE_DIR}/include)


if(FENRIR_USE_OPENCL)
    find_package(OpenCL QUIET)
    if(NOT OpenCL_FOUND)
        list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
        find_package(OpenCL REQUIRED)
        message(STATUS "OpenCL found via custom FindOpenCL.cmake")
    else()
        message(STATUS "OpenCL found: ${OpenCL_INCLUDE_DIR}")
    endif()
endif()

if(FENRIR_USE_CURL)
    find_package(CURL QUIET)
    if(NOT CURL_FOUND)
        message(WARNING "libcurl not found — API attack mode will be disabled")
        set(FENRIR_USE_CURL OFF)
    else()
        message(STATUS "libcurl found: ${CURL_VERSION_STRING}")
    endif()
endif()

add_library(fenrir_core STATIC
    src/core/Config.cpp
    src/core/TargetHash.cpp
    src/core/HashEngineFactory.cpp
    src/core/MD5Engine.cpp
    src/core/SHA1Engine.cpp
    src/core/SHA256Engine.cpp
    src/core/SHA512Engine.cpp
    src/core/NTMLEngine.cpp
    src/core/BCryptEngine.cpp
    src/core/SCryptEngine.cpp
    src/core/SHA3Engine.cpp
    src/core/PBKDF2Engine.cpp
    src/core/Argon2Engine.cpp
    src/core/SHA384Engine.cpp
    src/core/Blake2bEngine.cpp
    src/core/HMACMD5Engine.cpp
    src/core/HMACSHA256Engine.cpp
    src/core/Pipeline.cpp
    src/core/ResultWriter.cpp
    src/core/Benchmark.cpp)
target_include_directories(fenrir_core PUBLIC src)
target_link_libraries(fenrir_core PUBLIC nlohmann_json::nlohmann_json spdlog::spdlog fenrir_argon2 fenrir_bcrypt)

add_library(fenrir_utils STATIC
    src/utils/Logger.cpp
    src/utils/FileReader.cpp
    src/utils/HashParser.cpp
    src/utils/ShadowParser.cpp
    src/utils/PlistParser.cpp
    src/utils/Checkpoint.cpp
    src/utils/SignalHandler.cpp
    src/ui/Terminal.cpp
    src/ui/ProgressDisplay.cpp)
target_include_directories(fenrir_utils PUBLIC src)
target_link_libraries(fenrir_utils PUBLIC fenrir_core)

add_library(fenrir_attack STATIC
    src/attack/DictionaryAttack.cpp
    src/attack/RuleBasedAttack.cpp
    src/attack/MaskAttack.cpp
    src/attack/HybridAttack.cpp
    src/attack/ApiAttack.cpp
    src/attack/CombinatorAttack.cpp
    src/attack/AttackRegistry.cpp)
target_include_directories(fenrir_attack PUBLIC src)
target_link_libraries(fenrir_attack PUBLIC fenrir_core)

add_library(fenrir_rules STATIC
    src/rules/RuleParser.cpp
    src/rules/RuleEngine.cpp)
target_include_directories(fenrir_rules PUBLIC src)
target_link_libraries(fenrir_rules PUBLIC fenrir_core)

add_library(fenrir_simd STATIC
    src/cpu_simd/SIMDDetector.cpp
    src/cpu_simd/MD5_AVX2.cpp
    src/cpu_simd/SHA256_AVX2.cpp)
target_include_directories(fenrir_simd PUBLIC src)
target_link_libraries(fenrir_simd PUBLIC fenrir_core)
if(MSVC)
    target_compile_options(fenrir_simd PRIVATE /arch:AVX2)
else()
    target_compile_options(fenrir_simd PRIVATE -mavx2 -mfma)
endif()

if(FENRIR_USE_OPENCL)
    add_library(fenrir_gpu STATIC
        src/gpu/OpenCLContext.cpp
        src/gpu/OpenCLKernel.cpp
        src/gpu/GpuHasher.cpp
        src/gpu/AsyncGpuHasher.cpp
        src/gpu/KernelCache.cpp
        src/gpu/GpuStats.cpp)
    target_include_directories(fenrir_gpu PUBLIC src)
    target_link_libraries(fenrir_gpu PUBLIC OpenCL::OpenCL fenrir_core)
    target_compile_definitions(fenrir_gpu PUBLIC FENRIR_HAS_GPU=1)
endif()

if(FENRIR_USE_CURL)
    add_library(fenrir_api STATIC
        src/api/LeakLookupProvider.cpp
        src/api/HashKillerProvider.cpp
        src/api/HashToolkitProvider.cpp
        src/api/Md5DecryptProvider.cpp
        src/api/ApiManager.cpp
        src/api/RateLimiter.cpp)
    target_include_directories(fenrir_api PUBLIC src)
    target_link_libraries(fenrir_api PUBLIC CURL::libcurl fenrir_core)
    target_compile_definitions(fenrir_api PUBLIC FENRIR_HAS_API=1)
endif()

add_library(fenrir_cli STATIC
    src/cli/CliParser.cpp)
target_include_directories(fenrir_cli PUBLIC src)
target_link_libraries(fenrir_cli PUBLIC CLI11::CLI11 fenrir_core)

add_executable(fenrir
    src/main.cpp)
target_include_directories(fenrir PRIVATE src)

target_link_libraries(fenrir PRIVATE
    fenrir_core
    fenrir_attack
    fenrir_rules
    fenrir_cli
    fenrir_utils
    fenrir_simd
    $<$<BOOL:${FENRIR_USE_OPENCL}>:fenrir_gpu>
    $<$<BOOL:${FENRIR_USE_CURL}>:fenrir_api>)

install(TARGETS fenrir RUNTIME DESTINATION bin)
install(DIRECTORY kernels/ DESTINATION share/fenrir/kernels)
install(DIRECTORY rules/   DESTINATION share/fenrir/rules)
install(FILES config/default_config.json DESTINATION share/fenrir/config)

if(FENRIR_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1)
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
    include(Catch)
    add_subdirectory(tests)
endif()
