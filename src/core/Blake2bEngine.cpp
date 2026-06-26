#include "Blake2bEngine.hpp"
#include <cstring>
#include <vector>

namespace fenrir { namespace core {

Blake2bEngine::Blake2bEngine(int digestBytes)
    : m_digestSize(digestBytes), m_name("Blake2b-" + std::to_string(digestBytes * 8)) {}

// BLAKE2b constants
static const uint64_t IV[8] = {
    0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
    0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};

static void blake2bCompress(uint64_t* h, const uint8_t* block, uint64_t t, bool last) {
    uint64_t v[16];
    for (int i = 0; i < 8; i++) v[i] = h[i];
    for (int i = 0; i < 8; i++) v[i + 8] = IV[i];
    v[12] ^= t;
    v[13] ^= 0;
    if (last) v[14] = ~v[14];

    uint64_t m[16];
    for (int i = 0; i < 16; i++)
        m[i] = ((uint64_t)block[i*8]) | ((uint64_t)block[i*8+1]<<8) |
               ((uint64_t)block[i*8+2]<<16) | ((uint64_t)block[i*8+3]<<24) |
               ((uint64_t)block[i*8+4]<<32) | ((uint64_t)block[i*8+5]<<40) |
               ((uint64_t)block[i*8+6]<<48) | ((uint64_t)block[i*8+7]<<56);

    static const int sigma[12][16] = {
        {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
        {14,10,4,8,9,15,13,6,1,12,0,2,11,7,5,3},
        {11,8,12,0,5,2,15,13,10,14,3,6,7,1,9,4},
        {7,9,3,1,13,12,11,14,2,6,5,10,4,0,15,8},
        {9,0,5,7,2,4,10,15,14,1,11,12,6,8,3,13},
        {2,12,6,10,0,11,8,3,4,13,7,5,15,14,1,9},
        {12,5,1,15,14,13,4,10,0,7,6,3,9,2,8,11},
        {13,11,7,14,12,1,3,9,5,0,15,4,8,6,2,10},
        {6,15,14,9,11,3,0,8,12,2,13,7,1,4,10,5},
        {10,2,8,4,7,6,1,5,15,11,9,14,3,12,13,0},
        {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
        {14,10,4,8,9,15,13,6,1,12,0,2,11,7,5,3}
    };

#define G(r,i,a,b,c,d) do { \
    a = a + b + m[sigma[r][2*i]]; d = (d ^ a) >> 32 | (d ^ a) << 32; \
    c = c + d; b = (b ^ c) >> 24 | (b ^ c) << 40; \
    a = a + b + m[sigma[r][2*i+1]]; d = (d ^ a) >> 16 | (d ^ a) << 48; \
    c = c + d; b = (b ^ c) >> 63 | (b ^ c) << 1; \
} while(0)

#define ROUND(r) do { \
    G(r,0,v[0],v[4],v[8],v[12]); G(r,1,v[1],v[5],v[9],v[13]); \
    G(r,2,v[2],v[6],v[10],v[14]); G(r,3,v[3],v[7],v[11],v[15]); \
    G(r,4,v[0],v[5],v[10],v[15]); G(r,5,v[1],v[6],v[11],v[12]); \
    G(r,6,v[2],v[7],v[8],v[13]); G(r,7,v[3],v[4],v[9],v[14]); \
} while(0)

    for (int r = 0; r < 12; r++) ROUND(r);

    for (int i = 0; i < 8; i++) h[i] ^= v[i] ^ v[i + 8];
#undef G
#undef ROUND
}

std::vector<uint8_t> Blake2bEngine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    const size_t blockSize = 128;
    uint64_t h[8];
    for (int i = 0; i < 8; i++) h[i] = IV[i] ^ (static_cast<uint64_t>(m_digestSize) | (static_cast<uint64_t>(salt.size()) << 8) | 0x01010000);

    size_t totalLen = input.size();
    size_t pos = 0;
    uint8_t block[128] = {};
    uint64_t t = 0;

    while (pos + blockSize <= totalLen) {
        blake2bCompress(h, reinterpret_cast<const uint8_t*>(input.data() + pos), t, false);
        pos += blockSize;
        t += blockSize;
    }

    size_t remaining = totalLen - pos;
    memcpy(block, input.data() + pos, remaining);
    blake2bCompress(h, block, t + remaining, true);

    std::vector<uint8_t> result(m_digestSize);
    for (int i = 0; i < m_digestSize && i / 8 < 8; i++) {
        result[i] = static_cast<uint8_t>(h[i / 8] >> ((i % 8) * 8));
    }
    return result;
}

void Blake2bEngine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (size_t i = 0; i < inputs.size(); i++) outputs[i] = hash(inputs[i], salt);
}

} }
