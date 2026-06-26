#include "PBKDF2Engine.hpp"
#include "SHA256Engine.hpp"
#include <cstring>

namespace fenrir { namespace core {

static void hmacSha256(const std::string& key, const std::vector<uint8_t>& data, uint8_t* out) {
    SHA256Engine sha;
    constexpr size_t B = 64;
    uint8_t kPad[B] = {0};
    if (key.size() > B) {
        auto h = sha.hash(key);
        memcpy(kPad, h.data(), std::min(h.size(), B));
    } else {
        memcpy(kPad, key.data(), key.size());
    }
    uint8_t iPad[B], oPad[B];
    for (size_t i = 0; i < B; i++) { iPad[i] = kPad[i] ^ 0x36; oPad[i] = kPad[i] ^ 0x5c; }
    std::vector<uint8_t> inner(B + data.size());
    memcpy(inner.data(), iPad, B);
    memcpy(inner.data() + B, data.data(), data.size());
    auto hInner = sha.hash(std::string(inner.begin(), inner.end()));
    std::vector<uint8_t> outer(B + hInner.size());
    memcpy(outer.data(), oPad, B);
    memcpy(outer.data() + B, hInner.data(), hInner.size());
    auto hmac = sha.hash(std::string(outer.begin(), outer.end()));
    memcpy(out, hmac.data(), 32);
}

PBKDF2Engine::PBKDF2Engine(int iterations, int keyLength)
    : m_iterations(iterations), m_keyLength(keyLength) {
    m_name = "PBKDF2-HMAC-SHA256:" + std::to_string(iterations);
}

std::vector<uint8_t> PBKDF2Engine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    std::vector<uint8_t> result(m_keyLength);
    size_t blocks = (m_keyLength + 31) / 32;
    uint8_t u[32], t[32];

    for (size_t block = 1; block <= blocks; block++) {
        std::vector<uint8_t> s(salt.begin(), salt.end());
        s.push_back((block >> 24) & 0xFF); s.push_back((block >> 16) & 0xFF);
        s.push_back((block >> 8) & 0xFF);  s.push_back(block & 0xFF);

        hmacSha256(input, s, u);
        memcpy(t, u, 32);

        for (int iter = 1; iter < m_iterations; iter++) {
            hmacSha256(input, std::vector<uint8_t>(u, u + 32), u);
            for (int j = 0; j < 32; j++) t[j] ^= u[j];
        }

        size_t copyLen = std::min((size_t)32, m_keyLength - (block - 1) * 32);
        memcpy(result.data() + (block - 1) * 32, t, copyLen);
    }
    return result;
}

void PBKDF2Engine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (size_t i = 0; i < inputs.size(); i++) outputs[i] = hash(inputs[i], salt);
}

double PBKDF2Engine::estimatedHashPerSecond(int gpuComputeUnits) const { return gpuComputeUnits * 1000.0 / m_iterations; }

} }
