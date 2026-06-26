#include "ShadowParser.hpp"
#include "FileReader.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace fenrir {
namespace utils {

static std::string lower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return r;
}

bool ShadowParser::isShadowLine(const std::string& line) {
    if (line.empty() || line[0] == '#') return false;
    // Format: user:hash:... (at least 2 colons)
    size_t c1 = line.find(':');
    if (c1 == std::string::npos) return false;
    size_t c2 = line.find(':', c1 + 1);
    return c2 != std::string::npos;
}

std::optional<ShadowEntry> ShadowParser::parseLine(const std::string& line) {
    if (!isShadowLine(line)) return std::nullopt;

    ShadowEntry entry;
    entry.rawLine = line;

    size_t c1 = line.find(':');
    size_t c2 = line.find(':', c1 + 1);

    entry.username = line.substr(0, c1);
    entry.hashString = line.substr(c1 + 1, c2 - c1 - 1);

    // Skip locked/disabled accounts (*, !)
    if (entry.hashString.empty() || entry.hashString == "*" || entry.hashString == "!") {
        return std::nullopt;
    }

    // If hash starts with ! (locked but hash preserved), strip the !
    if (entry.hashString[0] == '!') {
        entry.hashString = entry.hashString.substr(1);
    }

    return entry;
}

std::vector<ShadowEntry> ShadowParser::parseFile(const std::string& filePath) {
    std::vector<ShadowEntry> entries;
    FileReader reader(filePath);
    int lineNum = 0;

    while (auto line = reader.nextLine()) {
        lineNum++;
        std::string trimmed = *line;
        size_t start = trimmed.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = trimmed.find_last_not_of(" \t\r\n");
        trimmed = trimmed.substr(start, end - start + 1);

        auto entry = parseLine(trimmed);
        if (entry) {
            entries.push_back(*entry);
            Logger::debug("Shadow: parsed user '" + entry->username +
                         "' with hash prefix " + entry->hashString.substr(0, std::min(size_t(3), entry->hashString.size())));
        }
    }

    Logger::info("ShadowParser: loaded " + std::to_string(entries.size()) +
                " user(s) from " + filePath);
    return entries;
}

std::string ShadowParser::detectHashMode(const std::string& hashString) {
    if (hashString.empty()) return "";

    std::string s = lower(hashString);

    // $id$ prefix detection
    if (s.size() >= 3 && s[0] == '$') {
        if (s[1] == '1') return "md5";           // $1$ = MD5
        if (s[1] == '5') return "sha256";        // $5$ = SHA256
        if (s[1] == '6') return "sha512";        // $6$ = SHA512
        if (s[1] == 'y') return "scrypt";        // $y$ = yescrypt
        if (s.find("$2a$") == 0 || s.find("$2b$") == 0 || s.find("$2y$") == 0)
            return "bcrypt";
        if (s.find("$argon2") == 0) return "argon2";
        if (s.find("$pbkdf2") == 0) return "pbkdf2";
        if (s.find("$7$") == 0) return "scrypt";
    }

    return "";
}

} // namespace utils
} // namespace fenrir
