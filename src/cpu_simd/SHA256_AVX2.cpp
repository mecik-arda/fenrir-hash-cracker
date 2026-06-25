#include "SHA256_AVX2.hpp"
#include "SIMDDetector.hpp"
#include <cstring>
#include <immintrin.h>
#include <algorithm>

namespace fenrir { namespace simd {

static inline __m256i sha256_rotr(__m256i x, int n) {
    return _mm256_or_si256(_mm256_srli_epi32(x, n), _mm256_slli_epi32(x, 32 - n));
}
static inline __m256i sha256_CH(__m256i x, __m256i y, __m256i z) {
    return _mm256_xor_si256(_mm256_and_si256(x, y), _mm256_andnot_si256(x, z));
}
static inline __m256i sha256_MAJ(__m256i x, __m256i y, __m256i z) {
    return _mm256_xor_si256(_mm256_xor_si256(_mm256_and_si256(x, y),
          _mm256_and_si256(x, z)), _mm256_and_si256(y, z));
}
static inline __m256i sha256_BSIG0(__m256i x) {
    return _mm256_xor_si256(_mm256_xor_si256(sha256_rotr(x, 2), sha256_rotr(x, 13)), sha256_rotr(x, 22));
}
static inline __m256i sha256_BSIG1(__m256i x) {
    return _mm256_xor_si256(_mm256_xor_si256(sha256_rotr(x, 6), sha256_rotr(x, 11)), sha256_rotr(x, 25));
}
static inline __m256i sha256_SSIG0(__m256i x) {
    return _mm256_xor_si256(_mm256_xor_si256(sha256_rotr(x, 7), sha256_rotr(x, 18)), _mm256_srli_epi32(x, 3));
}
static inline __m256i sha256_SSIG1(__m256i x) {
    return _mm256_xor_si256(_mm256_xor_si256(sha256_rotr(x, 17), sha256_rotr(x, 19)), _mm256_srli_epi32(x, 10));
}

static const uint32_t SHA256_K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,
    0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,
    0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,
    0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,
    0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,
    0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_avx2_8way(const uint8_t* msgs[8], const size_t lens[8], uint8_t* digests[8]) {
    __m256i H[8];
    H[0] = _mm256_set1_epi32(0x6a09e667); H[1] = _mm256_set1_epi32(0xbb67ae85);
    H[2] = _mm256_set1_epi32(0x3c6ef372); H[3] = _mm256_set1_epi32(0xa54ff53a);
    H[4] = _mm256_set1_epi32(0x510e527f); H[5] = _mm256_set1_epi32(0x9b05688c);
    H[6] = _mm256_set1_epi32(0x1f83d9ab); H[7] = _mm256_set1_epi32(0x5be0cd19);

    uint8_t padBufs[8][128];
    size_t padLens[8];
    for (int lane = 0; lane < 8; lane++) {
        size_t ml = lens[lane];
        uint64_t bitLen = ml * 8;
        memcpy(padBufs[lane], msgs[lane], ml);
        padBufs[lane][ml] = 0x80;
        padLens[lane] = ((ml % 64) < 56) ? ((ml / 64) + 1) * 64 : ((ml / 64) + 2) * 64;
        for (size_t i = ml + 1; i < padLens[lane] - 8; i++) padBufs[lane][i] = 0;
        for (int i = 7; i >= 0; i--) padBufs[lane][padLens[lane] - 8 + i] = (bitLen >> ((7 - i) * 8)) & 0xFF;
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
                    M_lanes[lane][i] = ((uint32_t)padBufs[lane][off + i*4] << 24) |
                        ((uint32_t)padBufs[lane][off + i*4 + 1] << 16) |
                        ((uint32_t)padBufs[lane][off + i*4 + 2] << 8) |
                        (uint32_t)padBufs[lane][off + i*4 + 3];
                }
            } else {
                memset(M_lanes[lane], 0, 64);
            }
        }

        __m256i W[64];
        for (int t = 0; t < 16; t++) {
            W[t] = _mm256_set_epi32(
                M_lanes[7][t], M_lanes[6][t], M_lanes[5][t], M_lanes[4][t],
                M_lanes[3][t], M_lanes[2][t], M_lanes[1][t], M_lanes[0][t]);
        }
        for (int t = 16; t < 64; t++) {
            W[t] = _mm256_add_epi32(_mm256_add_epi32(sha256_SSIG1(W[t-2]), W[t-7]),
                    _mm256_add_epi32(sha256_SSIG0(W[t-15]), W[t-16]));
        }

        __m256i a = H[0], b = H[1], c = H[2], d = H[3];
        __m256i e = H[4], f = H[5], g = H[6], h = H[7];

        for (int t = 0; t < 64; t++) {
            __m256i kk = _mm256_set1_epi32(SHA256_K[t]);
            __m256i t1 = _mm256_add_epi32(_mm256_add_epi32(_mm256_add_epi32(h, sha256_BSIG1(e)),
                        sha256_CH(e, f, g)), _mm256_add_epi32(kk, W[t]));
            __m256i t2 = _mm256_add_epi32(sha256_BSIG0(a), sha256_MAJ(a, b, c));
            h = g; g = f; f = e;
            e = _mm256_add_epi32(d, t1);
            d = c; c = b; b = a;
            a = _mm256_add_epi32(t1, t2);
        }

        H[0] = _mm256_add_epi32(H[0], a); H[1] = _mm256_add_epi32(H[1], b);
        H[2] = _mm256_add_epi32(H[2], c); H[3] = _mm256_add_epi32(H[3], d);
        H[4] = _mm256_add_epi32(H[4], e); H[5] = _mm256_add_epi32(H[5], f);
        H[6] = _mm256_add_epi32(H[6], g); H[7] = _mm256_add_epi32(H[7], h);
    }

    uint32_t out[8][8];
    for (int i = 0; i < 8; i++) _mm256_storeu_si256((__m256i*)out[i], H[i]);
    for (int lane = 0; lane < 8; lane++) {
        for (int i = 0; i < 8; i++) {
            uint8_t* o = digests[lane] + i * 4;
            uint32_t v = out[i][lane];
            o[0] = v >> 24; o[1] = v >> 16; o[2] = v >> 8; o[3] = v;
        }
    }
}

std::vector<uint8_t> SHA256_AVX2_Engine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    if (!SIMDDetector::hasAVX2()) return {};
    std::string s = salt.empty() ? input : std::string(salt.begin(), salt.end()) + input;
    const uint8_t* msgs[8] = { (const uint8_t*)s.data() };
    size_t lens[8] = { s.size() };
    uint8_t buf[32], *digests[8] = { buf };
    sha256_avx2_8way(msgs, lens, digests);
    return std::vector<uint8_t>(buf, buf + 32);
}

void SHA256_AVX2_Engine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    if (!SIMDDetector::hasAVX2() || inputs.empty()) { for (auto& o : outputs) o.resize(32, 0); return; }
    for (size_t base = 0; base < inputs.size(); base += 8) {
        size_t batchEnd = std::min(base + 8, inputs.size());
        size_t count = batchEnd - base;
        const uint8_t* msgs[8] = {}; size_t lens[8] = {}; uint8_t db[8][32]; uint8_t* digs[8] = {};
        for (size_t i = 0; i < count; i++) {
            std::string s = salt.empty() ? inputs[base + i] : std::string(salt.begin(), salt.end()) + inputs[base + i];
            msgs[i] = (const uint8_t*)s.data(); lens[i] = s.size(); digs[i] = db[i];
        }
        for (size_t i = count; i < 8; i++) { msgs[i] = (const uint8_t*)""; lens[i] = 0; digs[i] = db[i]; }
        sha256_avx2_8way(msgs, lens, digs);
        for (size_t i = 0; i < count; i++) outputs[base + i] = std::vector<uint8_t>(db[i], db[i] + 32);
    }
}
double SHA256_AVX2_Engine::estimatedHashPerSecond(int) const { return 60'000'000.0; }

} }
