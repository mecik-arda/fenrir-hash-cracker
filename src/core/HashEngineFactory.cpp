#include "HashEngineFactory.hpp"
#include "MD5Engine.hpp"
#include "SHA1Engine.hpp"
#include "SHA256Engine.hpp"
#include "SHA512Engine.hpp"
#include "NTMLEngine.hpp"
#include "BCryptEngine.hpp"
#include "SCryptEngine.hpp"
#include "SHA3Engine.hpp"
#include "PBKDF2Engine.hpp"
#include "Argon2Engine.hpp"
#include "SHA384Engine.hpp"
#include "Blake2bEngine.hpp"
#include "HMACMD5Engine.hpp"
#include "HMACSHA256Engine.hpp"
#include "../cpu_simd/MD5_AVX2.hpp"
#include "../cpu_simd/SHA256_AVX2.hpp"
#include "../cpu_simd/SIMDDetector.hpp"

#include <algorithm>
#include <stdexcept>
#include <cctype>

namespace fenrir {
namespace core {

std::unique_ptr<IHashEngine> HashEngineFactory::create(HashType type) {
    switch (type) {
        case HashType::MD5:    return std::make_unique<MD5Engine>();
        case HashType::SHA1:   return std::make_unique<SHA1Engine>();
        case HashType::SHA256: return std::make_unique<SHA256Engine>();
        case HashType::SHA384: return std::make_unique<SHA384Engine>();
        case HashType::SHA512: return std::make_unique<SHA512Engine>();
        case HashType::NTLM:   return std::make_unique<NTMLEngine>();
        case HashType::BCRYPT: return std::make_unique<BCryptEngine>();
        case HashType::SCRYPT: return std::make_unique<SCryptEngine>();
        case HashType::SHA3:   return std::make_unique<SHA3Engine>(256);
        case HashType::PBKDF2: return std::make_unique<PBKDF2Engine>(10000, 32);
        case HashType::ARGON2: return std::make_unique<Argon2Engine>(65536, 3, 4);
        case HashType::BLAKE2B:    return std::make_unique<Blake2bEngine>(64);
        case HashType::HMAC_MD5:   return std::make_unique<HMACMD5Engine>();
        case HashType::HMAC_SHA256: return std::make_unique<HMACSHA256Engine>();
        default: return nullptr;
    }
}

std::unique_ptr<IHashEngine> HashEngineFactory::create(const std::string& name, bool useSIMD) {
    if (useSIMD && simd::SIMDDetector::hasAVX2()) {
        std::string lower = name;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
        if (lower == "md5")    return std::make_unique<simd::MD5_AVX2_Engine>();
        if (lower == "sha256") return std::make_unique<simd::SHA256_AVX2_Engine>();
    }
    return create(typeFromName(name));
}

HashType HashEngineFactory::typeFromName(const std::string& name) {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

    if (lower == "md5")             return HashType::MD5;
    if (lower == "sha1")            return HashType::SHA1;
    if (lower == "sha256")          return HashType::SHA256;
    if (lower == "sha512")          return HashType::SHA512;
    if (lower == "ntlm")            return HashType::NTLM;
    if (lower == "bcrypt")          return HashType::BCRYPT;
    if (lower == "scrypt")          return HashType::SCRYPT;
    if (lower == "sha3" || lower == "sha3-256") return HashType::SHA3;
    if (lower == "pbkdf2" || lower == "pbkdf2-hmac-sha256") return HashType::PBKDF2;
    if (lower == "argon2" || lower == "argon2id") return HashType::ARGON2;
    if (lower == "sha384")           return HashType::SHA384;
    if (lower == "blake2b" || lower == "blake2" || lower == "blake2b-512")
        return HashType::BLAKE2B;
    if (lower == "hmac-md5" || lower == "hmacmd5")
        return HashType::HMAC_MD5;
    if (lower == "hmac-sha256" || lower == "hmacsha256")
        return HashType::HMAC_SHA256;
    throw std::runtime_error("Unknown hash type: " + name);
}

}
}
