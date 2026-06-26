#pragma once

#include <string>
#include <vector>
#include <optional>

namespace fenrir {
namespace utils {

struct PlistEntry {
    std::string username;
    std::string hashString;  // Base64 PBKDF2 hash data
};

class PlistParser {
public:
    /// Detect if a string looks like macOS plist user data
    static bool isPlistContent(const std::string& content);

    /// Extract username from plist filename (username.plist)
    static std::string usernameFromPath(const std::string& filePath);

    /// Parse plist content looking for PBKDF2/SHA512 hash entries
    static std::vector<PlistEntry> parseContent(const std::string& content,
                                                  const std::string& username);

    /// Parse a .plist file
    static std::vector<PlistEntry> parseFile(const std::string& filePath);
};

} // namespace utils
} // namespace fenrir
