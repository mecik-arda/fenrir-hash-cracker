#include "HashParser.hpp"
#include "FileReader.hpp"
#include "Logger.hpp"
#include "../core/Constants.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace fenrir {
namespace utils {

static std::string toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return r;
}

static std::vector<uint8_t> hexDecode(const std::string& hex) {
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        unsigned int b;
        std::stringstream ss;
        ss << std::hex << hex.substr(i, 2);
        ss >> b;
        bytes.push_back(static_cast<uint8_t>(b));
    }
    return bytes;
}

static std::string hexEncode(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto b : bytes) ss << std::setw(2) << static_cast<int>(b);
    return ss.str();
}

core::HashType HashParser::detectType(const std::string& hashStr) {
    std::string s = hashStr;


    if (!s.empty() && s[0] == '$') {
        if (s.size() > 4 && s[1] == '2' && (s[2] == 'a' || s[2] == 'b' || s[2] == 'y') && s[3] == '$')
            return core::HashType::BCRYPT;
        if (s.size() > 3 && s.substr(0, 3) == "$7$")
            return core::HashType::SCRYPT;
        if (s.size() > 8 && s.substr(0, 9) == "$argon2id$")
            return core::HashType::ARGON2;
        if (s.size() > 7 && s.substr(0, 8) == "$argon2i$")
            return core::HashType::ARGON2;
        if (s.size() > 7 && s.substr(0, 8) == "$pbkdf2-$")
            return core::HashType::PBKDF2;
    }


    std::string hashPart = s;
    size_t colonPos = s.find(':');
    if (colonPos != std::string::npos) {
        hashPart = s.substr(0, colonPos);
    }


    if (!std::all_of(hashPart.begin(), hashPart.end(),
                     [](char c) { return std::isxdigit(c); })) {
        return core::HashType::MD5;
    }


    size_t len = hashPart.length();
    switch (len) {
        case 32:  return core::HashType::MD5;
        case 40:  return core::HashType::SHA1;
        case 64:  return core::HashType::SHA256;
        case 128: return core::HashType::SHA512;
        default:  return core::HashType::MD5;
    }
}

core::TargetHash HashParser::parse(const std::string& hashStr,
                                     core::HashType expectedType) {
    std::string s = hashStr;


    if (expectedType == core::HashType::BCRYPT && !s.empty() && s[0] == '$') {
        std::vector<uint8_t> allBytes(s.begin(), s.end());
        return core::TargetHash(expectedType, allBytes, allBytes);
    }

    if (expectedType == core::HashType::ARGON2 && !s.empty() && s[0] == '$') {
        std::vector<uint8_t> allBytes(s.begin(), s.end());
        return core::TargetHash(expectedType, allBytes, allBytes);
    }

    if (expectedType == core::HashType::PBKDF2 && !s.empty() && s[0] == '$') {
        std::string rest = s.substr(s.find('$', 1) + 1);
        std::vector<uint8_t> hashBytes(rest.begin(), rest.end());
        return core::TargetHash(expectedType, hashBytes, {});
    }

    if (expectedType == core::HashType::SHA3) {
        size_t colonPos = s.find(':');
        std::string hashHex = (colonPos != std::string::npos) ? s.substr(0, colonPos) : s;
        std::vector<uint8_t> saltBytes;
        if (colonPos != std::string::npos)
            saltBytes.assign(s.begin() + colonPos + 1, s.end());
        std::vector<uint8_t> hashBytes = hexDecode(toLower(hashHex));
        return core::TargetHash(expectedType, hashBytes, saltBytes);
    }

    if (expectedType == core::HashType::SCRYPT && !s.empty() && s[0] == '$') {

        std::string rest = s.substr(3);
        std::vector<uint8_t> hashBytes(rest.begin(), rest.end());
        return core::TargetHash(expectedType, hashBytes, {});
    }


    size_t colonPos = s.find(':');
    std::string hashHex;
    std::vector<uint8_t> saltBytes;

    if (colonPos != std::string::npos) {
        hashHex = s.substr(0, colonPos);
        std::string saltStr = s.substr(colonPos + 1);
        saltBytes.assign(saltStr.begin(), saltStr.end());
    } else {
        hashHex = s;
    }


    std::vector<uint8_t> hashBytes = hexDecode(
        expectedType == core::HashType::NTLM ? toLower(hashHex) : toLower(hashHex));

    return core::TargetHash(expectedType, hashBytes, saltBytes);
}

core::TargetHash HashParser::parseAuto(const std::string& hashStr) {
    auto type = detectType(hashStr);
    return parse(hashStr, type);
}

std::vector<core::TargetHash> HashParser::parseFile(
    const std::string& filePath,
    core::HashType expectedType) {
    std::vector<core::TargetHash> targets;
    FileReader reader(filePath);
    int lineNum = 0;

    while (auto line = reader.nextLine()) {
        lineNum++;
        std::string trimmed = *line;

        size_t start = trimmed.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = trimmed.find_last_not_of(" \t\r\n");
        trimmed = trimmed.substr(start, end - start + 1);


        if (trimmed.empty() || trimmed[0] == '#') continue;

        try {
            auto target = parse(trimmed, expectedType);
            targets.push_back(target);
        } catch (const std::exception& e) {
            Logger::warn("Skipping line " + std::to_string(lineNum) +
                        " — parse error: " + e.what());
        }
    }
    return targets;
}

}
}
