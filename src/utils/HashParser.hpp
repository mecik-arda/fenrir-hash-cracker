#pragma once

#include "../core/TargetHash.hpp"
#include <string>
#include <vector>
#include <optional>

namespace fenrir {
namespace utils {

class HashParser {
public:

    static core::HashType detectType(const std::string& hashStr);


    static core::TargetHash parse(const std::string& hashStr,
                                   core::HashType expectedType);


    static core::TargetHash parseAuto(const std::string& hashStr);


    static std::vector<core::TargetHash> parseFile(
        const std::string& filePath,
        core::HashType expectedType);
};

}
}
