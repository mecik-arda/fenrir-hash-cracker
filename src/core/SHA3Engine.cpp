#include "SHA3Engine.hpp"
#include <cstring>
#include <algorithm>

namespace fenrir { namespace core {

static const uint64_t KECCAK_RC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

static const int KECCAK_ROT[25] = {
    0,1,62,28,27,36,44,6,55,20,3,10,43,25,39,41,45,15,21,8,18,2,61,56,14
};

static inline uint64_t rotl64(uint64_t x, int n) { return (x << n) | (x >> (64 - n)); }

static void keccak_f1600(uint64_t state[25]) {
    for (int round = 0; round < 24; round++) {
        uint64_t C[5], D[5];
        for (int x = 0; x < 5; x++) C[x] = state[x] ^ state[5+x] ^ state[10+x] ^ state[15+x] ^ state[20+x];
        for (int x = 0; x < 5; x++) D[x] = C[(x+4)%5] ^ rotl64(C[(x+1)%5], 1);
        for (int x = 0; x < 25; x++) state[x] ^= D[x % 5];

        uint64_t B[25];
        for (int x = 0; x < 5; x++)
            for (int y = 0; y < 5; y++)
                B[y*5 + ((2*x + 3*y) % 5)] = rotl64(state[x*5 + y], KECCAK_ROT[x*5 + y]);

        for (int x = 0; x < 5; x++)
            for (int y = 0; y < 5; y++)
                state[x*5 + y] = B[x*5 + y] ^ (~B[((x+1)%5)*5 + y] & B[((x+2)%5)*5 + y]);

        state[0] ^= KECCAK_RC[round];
    }
}

SHA3Engine::SHA3Engine(int bitStrength) : m_bitStrength(bitStrength) {
    m_name = "SHA3-" + std::to_string(bitStrength);
}

std::vector<uint8_t> SHA3Engine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    std::string s = salt.empty() ? input : std::string(salt.begin(), salt.end()) + input;
    int rate = 1600 - m_bitStrength * 2;
    int rateBytes = rate / 8;
    int outputBytes = m_bitStrength / 8;

    uint64_t state[25] = {0};
    const uint8_t* data = (const uint8_t*)s.data();
    size_t dataLen = s.size();
    size_t pos = 0;

    while (pos + rateBytes <= dataLen) {
        for (int i = 0; i < rateBytes / 8; i++) {
            uint64_t v = 0;
            for (int j = 0; j < 8; j++) v |= ((uint64_t)data[pos + i*8 + j]) << (j * 8);
            state[i] ^= v;
        }
        keccak_f1600(state);
        pos += rateBytes;
    }

    uint8_t pad[200] = {0};
    size_t remaining = dataLen - pos;
    memcpy(pad, data + pos, remaining);
    pad[remaining] = 0x06;
    pad[rateBytes - 1] |= 0x80;

    for (int i = 0; i < rateBytes / 8; i++) {
        uint64_t v = 0;
        for (int j = 0; j < 8; j++) v |= ((uint64_t)pad[i*8 + j]) << (j * 8);
        state[i] ^= v;
    }
    keccak_f1600(state);

    std::vector<uint8_t> result(outputBytes);
    size_t outPos = 0;
    while (outPos < outputBytes) {
        for (int i = 0; i < rateBytes / 8 && outPos < outputBytes; i++) {
            for (int j = 0; j < 8 && outPos < outputBytes; j++) {
                result[outPos++] = (state[i] >> (j * 8)) & 0xFF;
            }
        }
        if (outPos < outputBytes) keccak_f1600(state);
    }
    result.resize(outputBytes);
    return result;
}

void SHA3Engine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (size_t i = 0; i < inputs.size(); i++) outputs[i] = hash(inputs[i], salt);
}

double SHA3Engine::estimatedHashPerSecond(int gpuComputeUnits) const { return gpuComputeUnits * 25'000'000.0; }

} }
