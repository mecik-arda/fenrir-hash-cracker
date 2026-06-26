#include "MD5Engine.hpp"

#include <cstring>
#include <algorithm>

namespace fenrir {
namespace core {

namespace {

constexpr uint32_t K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

constexpr int S[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

inline uint32_t rotl32(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32 - n));
}

inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) | (~x & z);
}
inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) {
    return (x & z) | (y & ~z);
}
inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) {
    return x ^ y ^ z;
}
inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) {
    return y ^ (x | ~z);
}

void md5Transform(const uint8_t* msg, std::size_t msgLen, uint8_t* digest) {

    uint64_t bitLen = static_cast<uint64_t>(msgLen) * 8;

    // Compute padded length first, then allocate safely
    std::size_t padLen;
    if ((msgLen % 64) < 56) {
        padLen = ((msgLen / 64) + 1) * 64;
    } else {
        padLen = ((msgLen / 64) + 2) * 64;
    }

    std::vector<uint8_t> buf(padLen, 0);
    std::memcpy(buf.data(), msg, msgLen);

    buf[msgLen] = 0x80;

    for (int i = 0; i < 8; i++) {
        buf[padLen - 8 + i] = static_cast<uint8_t>(bitLen >> (i * 8));
    }


    uint32_t a = 0x67452301;
    uint32_t b = 0xefcdab89;
    uint32_t c = 0x98badcfe;
    uint32_t d = 0x10325476;


    for (std::size_t block = 0; block < padLen; block += 64) {

        uint32_t M[16];
        for (int i = 0; i < 16; i++) {
            M[i] = static_cast<uint32_t>(buf[block + i * 4]) |
                   (static_cast<uint32_t>(buf[block + i * 4 + 1]) << 8) |
                   (static_cast<uint32_t>(buf[block + i * 4 + 2]) << 16) |
                   (static_cast<uint32_t>(buf[block + i * 4 + 3]) << 24);
        }

        uint32_t aa = a, bb = b, cc = c, dd = d;


        auto step1 = [&](int i, int g) {
            uint32_t f = F(bb, cc, dd);
            uint32_t temp = aa + f + M[g] + K[i];
            aa = dd; dd = cc; cc = bb;
            bb = bb + rotl32(temp, S[i]);
        };
        step1( 0, 0); step1( 1, 1); step1( 2, 2); step1( 3, 3);
        step1( 4, 4); step1( 5, 5); step1( 6, 6); step1( 7, 7);
        step1( 8, 8); step1( 9, 9); step1(10, 10); step1(11, 11);
        step1(12, 12); step1(13, 13); step1(14, 14); step1(15, 15);


        auto step2 = [&](int i, int g) {
            uint32_t f = G(bb, cc, dd);
            uint32_t temp = aa + f + M[g] + K[i];
            aa = dd; dd = cc; cc = bb;
            bb = bb + rotl32(temp, S[i]);
        };
        step2(16, 1); step2(17, 6); step2(18, 11); step2(19, 0);
        step2(20, 5); step2(21, 10); step2(22, 15); step2(23, 4);
        step2(24, 9); step2(25, 14); step2(26, 3); step2(27, 8);
        step2(28, 13); step2(29, 2); step2(30, 7); step2(31, 12);


        auto step3 = [&](int i, int g) {
            uint32_t f = H(bb, cc, dd);
            uint32_t temp = aa + f + M[g] + K[i];
            aa = dd; dd = cc; cc = bb;
            bb = bb + rotl32(temp, S[i]);
        };
        step3(32, 5); step3(33, 8); step3(34, 11); step3(35, 14);
        step3(36, 1); step3(37, 4); step3(38, 7); step3(39, 10);
        step3(40, 13); step3(41, 0); step3(42, 3); step3(43, 6);
        step3(44, 9); step3(45, 12); step3(46, 15); step3(47, 2);


        auto step4 = [&](int i, int g) {
            uint32_t f = I(bb, cc, dd);
            uint32_t temp = aa + f + M[g] + K[i];
            aa = dd; dd = cc; cc = bb;
            bb = bb + rotl32(temp, S[i]);
        };
        step4(48, 0); step4(49, 7); step4(50, 14); step4(51, 5);
        step4(52, 12); step4(53, 3); step4(54, 10); step4(55, 1);
        step4(56, 8); step4(57, 15); step4(58, 6); step4(59, 13);
        step4(60, 4); step4(61, 11); step4(62, 2); step4(63, 9);

        a += aa; b += bb; c += cc; d += dd;
    }


    auto write32 = [&](uint8_t* out, uint32_t v) {
        out[0] = v & 0xFF; out[1] = (v >> 8) & 0xFF;
        out[2] = (v >> 16) & 0xFF; out[3] = (v >> 24) & 0xFF;
    };
    write32(digest + 0, a);
    write32(digest + 4, b);
    write32(digest + 8, c);
    write32(digest + 12, d);
}

}

std::vector<uint8_t> MD5Engine::hash(const std::string& input,
                                      const std::vector<uint8_t>& salt) const {

    std::string salted = input;
    if (!salt.empty()) {
        salted = std::string(salt.begin(), salt.end()) + input;
    }

    std::vector<uint8_t> result(16);
    md5Transform(reinterpret_cast<const uint8_t*>(salted.data()),
                 salted.size(), result.data());
    return result;
}

void MD5Engine::hashBatch(const std::vector<std::string>& inputs,
                           std::vector<std::vector<uint8_t>>& outputs,
                           const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (std::size_t i = 0; i < inputs.size(); ++i) {
        outputs[i] = hash(inputs[i], salt);
    }
}

double MD5Engine::estimatedHashPerSecond(int gpuComputeUnits) const {

    return static_cast<double>(gpuComputeUnits) * 50'000'000.0;
}

}
}
