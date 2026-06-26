#include "SIMDDetector.hpp"
#include <cstring>

#ifdef _WIN32
    #include <intrin.h>
#else
    #include <cpuid.h>
#endif

namespace fenrir { namespace simd {

SIMDLevel SIMDDetector::s_cached = SIMDLevel::None;
bool SIMDDetector::s_detected = false;

#if defined(_MSC_VER)
void SIMDDetector::cpuid(int info[4], int level) {
    __cpuidex(info, level, 0);
}
#else
void SIMDDetector::cpuid(int info[4], int level) {
    __cpuid_count(level, 0, info[0], info[1], info[2], info[3]);
}
#endif

bool SIMDDetector::checkXCR0() {
    #if defined(_MSC_VER)
        int64_t xcr0 = _xgetbv(0);
        // Check XMM (bit 1), YMM (bit 2), OPMASK (bit 5), ZMM_Hi256 (bit 6), ZMM_Hi16 (bit 7)
        return (xcr0 & 6) == 6;
        // For full AVX-512: (xcr0 & 0xE6) == 0xE6;
    #elif defined(__GNUC__) || defined(__clang__)
        uint64_t xcr0;
        __asm__ volatile("xgetbv" : "=a"(xcr0) : "c"(0) : "%edx");
        return (xcr0 & 6) == 6;
        // For full AVX-512: (xcr0 & 0xE6) == 0xE6;
    #else
        return false;
    #endif
}

bool SIMDDetector::checkXCR0_AVX512() {
    #if defined(_MSC_VER)
        int64_t xcr0 = _xgetbv(0);
        return (xcr0 & 0xE6) == 0xE6;  // Bits 1,2,5,6,7
    #elif defined(__GNUC__) || defined(__clang__)
        uint64_t xcr0;
        __asm__ volatile("xgetbv" : "=a"(xcr0) : "c"(0) : "%edx");
        return (xcr0 & 0xE6) == 0xE6;
    #else
        return false;
    #endif
}

SIMDLevel SIMDDetector::detect() {
    if (s_detected) return s_cached;
    s_detected = true;

    int info[4] = {0};
    cpuid(info, 0);
    int maxLevel = info[0];

    cpuid(info, 1);
    bool sse2   = (info[3] & (1 << 26)) != 0;
    bool sse4_1 = (info[2] & (1 << 19)) != 0;
    bool avx    = (info[2] & (1 << 28)) != 0;
    bool xsave  = (info[2] & (1 << 27)) != 0;

    if (!sse2) { s_cached = SIMDLevel::None; return s_cached; }
    if (!sse4_1) { s_cached = SIMDLevel::SSE2; return s_cached; }
    if (!avx || !xsave || !checkXCR0()) { s_cached = SIMDLevel::SSE4_1; return s_cached; }

    // Check AVX-512 with proper XCR0 bits
    cpuid(info, 7);
    bool avx2   = (info[1] & (1 << 5)) != 0;
    bool avx512 = (info[1] & (1 << 16)) != 0 &&
                  (info[1] & (1 << 17)) != 0 &&
                  (info[1] & (1 << 28)) != 0 &&
                  (info[1] & (1 << 30)) != 0 &&
                  (info[1] & (1 << 31)) != 0;

    if (avx512 && checkXCR0_AVX512()) s_cached = SIMDLevel::AVX512;
    else if (avx2)   s_cached = SIMDLevel::AVX2;
    else              s_cached = SIMDLevel::AVX;

    return s_cached;
}

bool SIMDDetector::hasAVX2()    { return detect() >= SIMDLevel::AVX2; }
bool SIMDDetector::hasAVX512()  { return detect() >= SIMDLevel::AVX512; }
bool SIMDDetector::hasSSE4_1()  { return detect() >= SIMDLevel::SSE4_1; }

std::string SIMDDetector::levelName(SIMDLevel level) {
    switch (level) {
        case SIMDLevel::AVX512: return "AVX-512";
        case SIMDLevel::AVX2:   return "AVX2";
        case SIMDLevel::AVX:    return "AVX";
        case SIMDLevel::SSE4_1: return "SSE4.1";
        case SIMDLevel::SSE2:   return "SSE2";
        default:                return "None";
    }
}

std::string SIMDDetector::levelName() { return levelName(detect()); }

} }
