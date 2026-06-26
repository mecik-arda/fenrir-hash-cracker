#include "SHA1Engine.hpp"
#include <cstring>

namespace fenrir { namespace core {

namespace {
constexpr uint32_t K[4] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6 };

inline uint32_t rotl32(uint32_t x, uint32_t n) { return (x << n) | (x >> (32 - n)); }
inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
inline uint32_t PARITY(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
inline uint32_t f0(uint32_t b, uint32_t c, uint32_t d) { return CH(b, c, d); }
inline uint32_t f1(uint32_t b, uint32_t c, uint32_t d) { return PARITY(b, c, d); }
inline uint32_t f2(uint32_t b, uint32_t c, uint32_t d) { return MAJ(b, c, d); }
inline uint32_t f3(uint32_t b, uint32_t c, uint32_t d) { return PARITY(b, c, d); }

void sha1Transform(const uint8_t* msg, std::size_t msgLen, uint8_t* digest) {
    uint64_t bitLen = static_cast<uint64_t>(msgLen) * 8;

    std::size_t padLen;
    if ((msgLen % 64) < 56) padLen = ((msgLen / 64) + 1) * 64;
    else padLen = ((msgLen / 64) + 2) * 64;

    std::vector<uint8_t> buf(padLen, 0);
    std::memcpy(buf.data(), msg, msgLen);
    buf[msgLen] = 0x80;

    for (int i = 7; i >= 0; i--) buf[padLen - 8 + i] = static_cast<uint8_t>(bitLen >> ((7 - i) * 8));

    uint32_t H0 = 0x67452301, H1 = 0xefcdab89, H2 = 0x98badcfe, H3 = 0x10325476, H4 = 0xc3d2e1f0;

    for (std::size_t block = 0; block < padLen; block += 64) {
        uint32_t W[80];
        for (int i = 0; i < 16; i++)
            W[i] = (uint32_t(buf[block + i*4]) << 24) | (uint32_t(buf[block + i*4 + 1]) << 16) |
                   (uint32_t(buf[block + i*4 + 2]) << 8) | uint32_t(buf[block + i*4 + 3]);
        for (int i = 16; i < 80; i++)
            W[i] = rotl32(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16], 1);

        uint32_t a = H0, b = H1, c = H2, d = H3, e = H4;

        for (int t = 0; t < 20; t++) {
            uint32_t temp = rotl32(a, 5) + f0(b, c, d) + e + W[t] + K[0];
            e = d; d = c; c = rotl32(b, 30); b = a; a = temp;
        }
        for (int t = 20; t < 40; t++) {
            uint32_t temp = rotl32(a, 5) + f1(b, c, d) + e + W[t] + K[1];
            e = d; d = c; c = rotl32(b, 30); b = a; a = temp;
        }
        for (int t = 40; t < 60; t++) {
            uint32_t temp = rotl32(a, 5) + f2(b, c, d) + e + W[t] + K[2];
            e = d; d = c; c = rotl32(b, 30); b = a; a = temp;
        }
        for (int t = 60; t < 80; t++) {
            uint32_t temp = rotl32(a, 5) + f3(b, c, d) + e + W[t] + K[3];
            e = d; d = c; c = rotl32(b, 30); b = a; a = temp;
        }

        H0 += a; H1 += b; H2 += c; H3 += d; H4 += e;
    }

    auto wb = [&](uint8_t* out, uint32_t v) {
        out[0] = v >> 24; out[1] = v >> 16; out[2] = v >> 8; out[3] = v;
    };
    wb(digest + 0, H0); wb(digest + 4, H1); wb(digest + 8, H2);
    wb(digest + 12, H3); wb(digest + 16, H4);
}
}

std::vector<uint8_t> SHA1Engine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    std::string s = salt.empty() ? input : std::string(salt.begin(), salt.end()) + input;
    std::vector<uint8_t> r(20);
    sha1Transform(reinterpret_cast<const uint8_t*>(s.data()), s.size(), r.data());
    return r;
}

void SHA1Engine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (std::size_t i = 0; i < inputs.size(); ++i) outputs[i] = hash(inputs[i], salt);
}

} }
