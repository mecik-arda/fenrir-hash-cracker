#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fenrir {
namespace core {

enum class HashType {
    MD5,
    SHA1,
    SHA256,
    SHA384,
    SHA512,
    NTLM,
    BCRYPT,
    SCRYPT,
    SHA3,
    PBKDF2,
    ARGON2,
    BLAKE2B,
    HMAC_MD5,
    HMAC_SHA256
};

class TargetHash {
public:
    TargetHash() = default;
    TargetHash(HashType algo, std::vector<uint8_t> hashBytes,
               std::vector<uint8_t> saltBytes = {});

    HashType algorithm() const;
    const std::vector<uint8_t>& hash() const;
    const std::vector<uint8_t>& salt() const;
    std::string hex() const;

    bool cracked = false;
    std::string username;  // populated when parsed from /etc/shadow

    void setUsername(const std::string& u) { username = u; }

private:
    HashType m_algorithm = HashType::MD5;
    std::vector<uint8_t> m_hash;
    std::vector<uint8_t> m_salt;
};

}
}
