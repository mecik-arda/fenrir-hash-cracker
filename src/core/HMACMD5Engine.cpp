#include "HMACMD5Engine.hpp"
#include "MD5Engine.hpp"
#include <cstring>

namespace fenrir { namespace core {

std::vector<uint8_t> HMACMD5Engine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    constexpr size_t B = 64; // MD5 block size
    uint8_t key[B] = {};

    // Use salt as HMAC key
    if (salt.size() > B) {
        MD5Engine md5;
        auto h = md5.hash(std::string(salt.begin(), salt.end()));
        memcpy(key, h.data(), std::min(h.size(), B));
    } else if (!salt.empty()) {
        memcpy(key, salt.data(), std::min(salt.size(), B));
    }

    uint8_t iPad[B], oPad[B];
    for (size_t i = 0; i < B; i++) { iPad[i] = key[i] ^ 0x36; oPad[i] = key[i] ^ 0x5C; }

    // Inner: MD5(iPad || input)
    std::string inner(reinterpret_cast<char*>(iPad), B);
    inner += input;
    MD5Engine md5;
    auto innerHash = md5.hash(inner);

    // Outer: MD5(oPad || innerHash)
    std::string outer(reinterpret_cast<char*>(oPad), B);
    outer.append(reinterpret_cast<const char*>(innerHash.data()), innerHash.size());
    return md5.hash(outer);
}

void HMACMD5Engine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (size_t i = 0; i < inputs.size(); i++) outputs[i] = hash(inputs[i], salt);
}

} }
