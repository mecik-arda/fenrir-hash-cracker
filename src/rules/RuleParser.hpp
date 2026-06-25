#pragma once

#include "RuleOpCodes.hpp"
#include <string>
#include <vector>

namespace fenrir {
namespace rules {

class RuleParser {
public:


    static std::vector<CompiledRule> parseFile(const std::string& path);


    static CompiledRule parseSingle(const std::string& ruleStr);
};

}
}
