#include "BCryptEngine.hpp"
#include <bcrypt/bcrypt.h>
#include <cstring>

namespace fenrir { namespace core {

std::vector<uint8_t> BCryptEngine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    if (salt.empty()) {
        return std::vector<uint8_t>(hashSize(), 0);
    }

    // Convert salt to string
    std::string saltStr(salt.begin(), salt.end());

    // bcrypt_hashpw expects only the "setting" (first 29 chars: $2a$10$[22-char-salt])
    // If we passed the full hash, extract just the setting portion
    if (saltStr.size() > 29 && saltStr[0] == '$') {
        // Find the 3rd '$' — the setting is everything up to that + 22 chars of salt
        // Simpler: first 29 chars is always the setting for 2a/2b/2y/$2$ variants
        saltStr = saltStr.substr(0, 29);
    }

    char outHash[BCRYPT_HASHSIZE];
    int rc = bcrypt_hashpw(input.c_str(), saltStr.c_str(), outHash);

    if (rc != 0) {
        return std::vector<uint8_t>(hashSize(), 0);
    }

    // outHash is a null-terminated string, copy it to vector
    std::string resultStr(outHash);
    return std::vector<uint8_t>(resultStr.begin(), resultStr.end());
}

void BCryptEngine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (size_t i = 0; i < inputs.size(); i++) {
        outputs[i] = hash(inputs[i], salt);
    }
}

} }
