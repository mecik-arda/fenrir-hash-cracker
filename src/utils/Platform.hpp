#pragma once

#include <cstdint>
#include <string>

#ifndef _WIN32
    #include <unistd.h>
#else
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#endif

namespace fenrir {
namespace utils {

enum class Platform {
    Windows,
    Linux,
    macOS,
    Unknown
};

#if defined(_WIN32) || defined(_WIN64)
    constexpr Platform CURRENT_PLATFORM = Platform::Windows;
#elif defined(__APPLE__) || defined(__MACH__)
    constexpr Platform CURRENT_PLATFORM = Platform::macOS;
#elif defined(__linux__) || defined(__linux) || defined(linux)
    constexpr Platform CURRENT_PLATFORM = Platform::Linux;
#else
    constexpr Platform CURRENT_PLATFORM = Platform::Unknown;
#endif

enum class Endian {
    Little,
    Big
};

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    constexpr Endian CURRENT_ENDIAN = Endian::Big;
#elif defined(_MSC_VER) || defined(__i386__) || defined(__x86_64__) || \
      defined(_M_IX86) || defined(_M_X64) || defined(__aarch64__)
    constexpr Endian CURRENT_ENDIAN = Endian::Little;
#else

    constexpr Endian CURRENT_ENDIAN = Endian::Little;
#endif

inline std::string platformName() {
    switch (CURRENT_PLATFORM) {
        case Platform::Windows: return "Windows";
        case Platform::Linux:   return "Linux";
        case Platform::macOS:   return "macOS";
        default:               return "Unknown";
    }
}

inline unsigned int cpuCoreCount() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return static_cast<unsigned int>(sysconf(_SC_NPROCESSORS_ONLN));
#endif
}

inline uint32_t swap32(uint32_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(x);
#elif defined(_MSC_VER)
    return _byteswap_ulong(x);
#else
    return (x >> 24) | ((x >> 8) & 0xFF00) | ((x << 8) & 0xFF0000) | (x << 24);
#endif
}

inline uint64_t swap64(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap64(x);
#elif defined(_MSC_VER)
    return _byteswap_uint64(x);
#else
    return (x >> 56) | ((x >> 40) & 0xFF00) |
           ((x >> 24) & 0xFF0000) | ((x >> 8) & 0xFF000000) |
           ((x << 8) & 0xFF00000000) | ((x << 24) & 0xFF0000000000) |
           ((x << 40) & 0xFF000000000000) | (x << 56);
#endif
}

}
}
