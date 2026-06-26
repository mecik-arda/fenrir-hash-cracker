#pragma once
#include <string>
#include <cstdint>

namespace fenrir { namespace simd {

enum class SIMDLevel {
    None   = 0,
    SSE2   = 1,
    SSE4_1 = 2,
    AVX    = 3,
    AVX2   = 4,
    AVX512 = 5
};

class SIMDDetector {
public:
    static SIMDLevel detect();

    static bool hasAVX2();
    static bool hasAVX512();
    static bool hasSSE4_1();

    static std::string levelName(SIMDLevel level);
    static std::string levelName();

private:
    static SIMDLevel s_cached;
    static bool s_detected;

    #if defined(_MSC_VER)
        static void cpuid(int info[4], int level);
    #elif defined(__GNUC__) || defined(__clang__)
        static void cpuid(int info[4], int level);
    #endif
    static bool checkXCR0();
    static bool checkXCR0_AVX512();
};

} }
