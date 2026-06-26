#include "MD5_AVX2.hpp"
#include "SIMDDetector.hpp"
#include <cstring>
#include <immintrin.h>
#include <algorithm>

namespace fenrir { namespace simd {

static inline __m256i md5_rotl32(__m256i x, int n) {
    return _mm256_or_si256(_mm256_slli_epi32(x, n), _mm256_srli_epi32(x, 32 - n));
}

static inline __m256i md5_F(__m256i x, __m256i y, __m256i z) {
    return _mm256_or_si256(_mm256_and_si256(x, y), _mm256_andnot_si256(x, z));
}
static inline __m256i md5_G(__m256i x, __m256i y, __m256i z) {
    return _mm256_or_si256(_mm256_and_si256(x, z), _mm256_andnot_si256(z, y));
}
static inline __m256i md5_H(__m256i x, __m256i y, __m256i z) {
    return _mm256_xor_si256(_mm256_xor_si256(x, y), z);
}
static inline __m256i md5_I(__m256i x, __m256i y, __m256i z) {
    return _mm256_xor_si256(y, _mm256_or_si256(x, _mm256_andnot_si256(z, _mm256_set1_epi32(-1))));
}

static const uint32_t MD5_K[64] = {
    0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
    0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
    0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
    0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
    0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
    0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
    0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
    0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};

static const int MD5_S[64] = {
    7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
    5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
    4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
    6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
};

static const int MD5_GIDX[64] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
    1,6,11,0,5,10,15,4,9,14,3,8,13,2,7,12,
    5,8,11,14,1,4,7,10,13,0,3,6,9,12,15,2,
    0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9
};

static void md5_avx2_8way(const uint8_t* msgs[8], const size_t lens[8], uint8_t* digests[8]) {
    __m256i a = _mm256_set_epi32(0x67452301,0x67452301,0x67452301,0x67452301,
                                  0x67452301,0x67452301,0x67452301,0x67452301);
    __m256i b = _mm256_set_epi32(0xefcdab89,0xefcdab89,0xefcdab89,0xefcdab89,
                                  0xefcdab89,0xefcdab89,0xefcdab89,0xefcdab89);
    __m256i c = _mm256_set_epi32(0x98badcfe,0x98badcfe,0x98badcfe,0x98badcfe,
                                  0x98badcfe,0x98badcfe,0x98badcfe,0x98badcfe);
    __m256i d = _mm256_set_epi32(0x10325476,0x10325476,0x10325476,0x10325476,
                                  0x10325476,0x10325476,0x10325476,0x10325476);

    uint64_t bitLens[8];
    size_t padLens[8];
    // Compute padLens first, then use max for a single dynamic buffer
    size_t maxPadLen = 0;
    for (int lane = 0; lane < 8; lane++) {
        size_t ml = lens[lane];
        bitLens[lane] = ml * 8;
        padLens[lane] = ((ml % 64) < 56) ? ((ml / 64) + 1) * 64 : ((ml / 64) + 2) * 64;
        if (padLens[lane] > maxPadLen) maxPadLen = padLens[lane];
    }
    std::vector<uint8_t> padBufsVec(maxPadLen * 8);
    uint8_t* padBufs[8];
    for (int lane = 0; lane < 8; lane++) {
        padBufs[lane] = padBufsVec.data() + lane * maxPadLen;
        size_t ml = lens[lane];
        memcpy(padBufs[lane], msgs[lane], ml);
        padBufs[lane][ml] = 0x80;
        for (size_t i = ml + 1; i < padLens[lane]; i++) padBufs[lane][i] = 0;
        uint64_t bl = bitLens[lane];
        for (int i = 0; i < 8; i++) padBufs[lane][padLens[lane] - 8 + i] = (bl >> (i * 8)) & 0xFF;
    }

    size_t maxBlocks = 0;
    for (int lane = 0; lane < 8; lane++) {
        size_t nb = padLens[lane] / 64;
        if (nb > maxBlocks) maxBlocks = nb;
    }

    for (size_t blockIdx = 0; blockIdx < maxBlocks; blockIdx++) {
        uint32_t M_lanes[8][16];
        for (int lane = 0; lane < 8; lane++) {
            size_t off = blockIdx * 64;
            if (off < padLens[lane]) {
                for (int i = 0; i < 16; i++) {
                    M_lanes[lane][i] = padBufs[lane][off + i*4] |
                        ((uint32_t)padBufs[lane][off + i*4 + 1] << 8) |
                        ((uint32_t)padBufs[lane][off + i*4 + 2] << 16) |
                        ((uint32_t)padBufs[lane][off + i*4 + 3] << 24);
                }
            } else {
                memset(M_lanes[lane], 0, 64);
            }
        }

        __m256i aa = a, bb = b, cc = c, dd = d;

        for (int r = 0; r < 64; r++) {
            int g = MD5_GIDX[r];
            __m256i mk = _mm256_set_epi32(
                M_lanes[7][g], M_lanes[6][g], M_lanes[5][g], M_lanes[4][g],
                M_lanes[3][g], M_lanes[2][g], M_lanes[1][g], M_lanes[0][g]);

            __m256i kk = _mm256_set1_epi32(MD5_K[r]);
            __m256i ss = _mm256_set1_epi32(MD5_S[r]);
            __m256i func;

            if (r < 16)      func = md5_F(bb, cc, dd);
            else if (r < 32) func = md5_G(bb, cc, dd);
            else if (r < 48) func = md5_H(bb, cc, dd);
            else             func = md5_I(bb, cc, dd);

            __m256i temp = _mm256_add_epi32(aa, func);
            temp = _mm256_add_epi32(temp, mk);
            temp = _mm256_add_epi32(temp, kk);

            uint32_t shifts[8];
            for (int i = 0; i < 8; i++) shifts[i] = MD5_S[r];
            __m256i sv = _mm256_loadu_si256((__m256i*)shifts);
            __m256i rot = _mm256_or_si256(_mm256_sllv_epi32(temp, sv),
                                           _mm256_srlv_epi32(temp, _mm256_sub_epi32(_mm256_set1_epi32(32), sv)));
            temp = _mm256_add_epi32(bb, rot);

            aa = dd;
            dd = cc;
            cc = bb;
            bb = temp;
        }

        a = _mm256_add_epi32(a, aa);
        b = _mm256_add_epi32(b, bb);
        c = _mm256_add_epi32(c, cc);
        d = _mm256_add_epi32(d, dd);
    }

    uint32_t A[8], B[8], C[8], D[8];
    _mm256_storeu_si256((__m256i*)A, a);
    _mm256_storeu_si256((__m256i*)B, b);
    _mm256_storeu_si256((__m256i*)C, c);
    _mm256_storeu_si256((__m256i*)D, d);

    for (int lane = 0; lane < 8; lane++) {
        uint8_t* out = digests[lane];
        out[0]  = A[lane] & 0xFF; out[1]  = (A[lane] >> 8) & 0xFF;
        out[2]  = (A[lane] >> 16) & 0xFF; out[3]  = (A[lane] >> 24) & 0xFF;
        out[4]  = B[lane] & 0xFF; out[5]  = (B[lane] >> 8) & 0xFF;
        out[6]  = (B[lane] >> 16) & 0xFF; out[7]  = (B[lane] >> 24) & 0xFF;
        out[8]  = C[lane] & 0xFF; out[9]  = (C[lane] >> 8) & 0xFF;
        out[10] = (C[lane] >> 16) & 0xFF; out[11] = (C[lane] >> 24) & 0xFF;
        out[12] = D[lane] & 0xFF; out[13] = (D[lane] >> 8) & 0xFF;
        out[14] = (D[lane] >> 16) & 0xFF; out[15] = (D[lane] >> 24) & 0xFF;
    }
}

std::vector<uint8_t> MD5_AVX2_Engine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    if (!SIMDDetector::hasAVX2()) {
        return {};
    }
    std::string s = salt.empty() ? input : std::string(salt.begin(), salt.end()) + input;
    // Use safe static buffers for unused lanes to avoid nullptr dereference
    static const uint8_t dummyMsg = 0;
    static uint8_t dummyDigest[16] = {};
    const uint8_t* msgs[8] = { (const uint8_t*)s.data(), &dummyMsg, &dummyMsg, &dummyMsg, &dummyMsg, &dummyMsg, &dummyMsg, &dummyMsg };
    size_t lens[8] = { s.size(), 0, 0, 0, 0, 0, 0, 0 };
    uint8_t buf[16];
    uint8_t d2[16], d3[16], d4[16], d5[16], d6[16], d7[16], d8[16];
    uint8_t* digests[8] = { buf, d2, d3, d4, d5, d6, d7, d8 };
    md5_avx2_8way(msgs, lens, digests);
    return std::vector<uint8_t>(buf, buf + 16);
}

void MD5_AVX2_Engine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    if (!SIMDDetector::hasAVX2() || inputs.empty()) {
        for (size_t i = 0; i < inputs.size(); i++) outputs[i].resize(16, 0);
        return;
    }

    for (size_t base = 0; base < inputs.size(); base += 8) {
        size_t batchEnd = std::min(base + 8, inputs.size());
        size_t count = batchEnd - base;

        const uint8_t* msgs[8] = {};
        size_t lens[8] = {};
        uint8_t digBufs[8][16];
        uint8_t* digests[8] = {};

        for (size_t i = 0; i < count; i++) {
            std::string s = salt.empty() ? inputs[base + i] : std::string(salt.begin(), salt.end()) + inputs[base + i];
            msgs[i] = (const uint8_t*)s.data();
            lens[i] = s.size();
            digests[i] = digBufs[i];
        }
        for (size_t i = count; i < 8; i++) {
            msgs[i] = (const uint8_t*)""; lens[i] = 0; digests[i] = digBufs[i];
        }

        md5_avx2_8way(msgs, lens, digests);
        for (size_t i = 0; i < count; i++) {
            outputs[base + i] = std::vector<uint8_t>(digBufs[i], digBufs[i] + 16);
        }
    }
}

double MD5_AVX2_Engine::estimatedHashPerSecond(int) const {
    return 80'000'000.0;
}

} }
