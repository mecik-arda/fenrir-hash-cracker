#pragma once

#include <string>
#include <vector>
#include <optional>

namespace fenrir {
namespace utils {

struct ShadowEntry {
    std::string username;
    std::string hashString;  // e.g. $6$salt$hash
    std::string rawLine;
};

class ShadowParser {
public:
    /// Detect if a line looks like /etc/shadow format
    static bool isShadowLine(const std::string& line);

    /// Parse a single shadow line into username + hash string
    static std::optional<ShadowEntry> parseLine(const std::string& line);

    /// Parse an entire /etc/shadow file into entries
    static std::vector<ShadowEntry> parseFile(const std::string& filePath);

    /// Detect hash type from shadow hash prefix ($1$, $5$, $6$, $y$, $2a$, etc.)
    static std::string detectHashMode(const std::string& hashString);
};

} // namespace utils
} // namespace fenrir
