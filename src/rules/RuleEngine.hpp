#pragma once

#include "RuleOpCodes.hpp"
#include <string>
#include <vector>
#include <optional>

namespace fenrir {
namespace rules {

class RuleEngine {
public:


    std::optional<std::string> apply(const CompiledRule& rule,
                                      const std::string& word);


    std::vector<std::string> applyAll(const std::vector<CompiledRule>& rules,
                                       const std::string& word);

private:


    int applyOp(OpCode op, uint8_t p0, uint8_t p1,
                char* buf, int len);

    char m_buffer[256]  = {};
    char m_memory[256]  = {};
    int  m_memoryLen    = 0;
};

}
}
