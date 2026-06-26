#include "PlistParser.hpp"
#include "FileReader.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <regex>

namespace fenrir {
namespace utils {

bool PlistParser::isPlistContent(const std::string& content) {
    return content.find("<plist") != std::string::npos ||
           content.find("<key>SALTED-SHA512-PBKDF2</key>") != std::string::npos ||
           content.find("<key>ShadowHashData</key>") != std::string::npos;
}

std::string PlistParser::usernameFromPath(const std::string& filePath) {
    // Extract "username" from ".../users/username.plist"
    size_t lastSlash = filePath.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos)
        ? filePath.substr(lastSlash + 1) : filePath;

    size_t dot = filename.rfind(".plist");
    if (dot != std::string::npos) {
        return filename.substr(0, dot);
    }
    return filename;
}

std::vector<PlistEntry> PlistParser::parseContent(const std::string& content,
                                                     const std::string& username) {
    std::vector<PlistEntry> entries;

    // Look for SALTED-SHA512-PBKDF2 entries (macOS 10.8+)
    std::regex entropyRx("<key>entropy</key>\\s*<data>\\s*([A-Za-z0-9+/=\\s]+)</data>");
    std::regex saltRx("<key>salt</key>\\s*<data>\\s*([A-Za-z0-9+/=\\s]+)</data>");
    std::regex iterationsRx("<key>iterations</key>\\s*<integer>(\\d+)</integer>");

    std::smatch match;

    std::string entropy, salt;
    int iterations = 0;

    if (std::regex_search(content, match, entropyRx)) {
        entropy = match[1].str();
        // Clean whitespace from base64
        entropy.erase(std::remove_if(entropy.begin(), entropy.end(), ::isspace), entropy.end());
    }
    if (std::regex_search(content, match, saltRx)) {
        salt = match[1].str();
        salt.erase(std::remove_if(salt.begin(), salt.end(), ::isspace), salt.end());
    }
    if (std::regex_search(content, match, iterationsRx)) {
        iterations = std::stoi(match[1].str());
    }

    if (!entropy.empty()) {
        PlistEntry entry;
        entry.username = username;
        // Format as: $pbkdf2-sha512$iterations$salt$entropy  (hashcat mode 7100 compatible)
        std::ostringstream hashStr;
        hashStr << "$pbkdf2-sha512$" << iterations << "$" << salt << "$" << entropy;
        entry.hashString = hashStr.str();
        entries.push_back(entry);
        Logger::debug("PlistParser: found PBKDF2 hash for user " + username);
    }

    return entries;
}

std::vector<PlistEntry> PlistParser::parseFile(const std::string& filePath) {
    std::vector<PlistEntry> entries;
    FileReader reader(filePath);
    std::string content;

    while (auto line = reader.nextLine()) {
        content += *line + "\n";
    }

    if (!isPlistContent(content)) return entries;

    std::string username = usernameFromPath(filePath);
    entries = parseContent(content, username);

    if (!entries.empty()) {
        Logger::info("PlistParser: extracted " + std::to_string(entries.size()) +
                    " hash(es) for user " + username);
    }
    return entries;
}

} // namespace utils
} // namespace fenrir
