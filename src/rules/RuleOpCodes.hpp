#pragma once

#include <cstdint>
#include <vector>

namespace fenrir {
namespace rules {

enum class OpCode : uint8_t {
    NOP      = ':',
    LOWER    = 'l',
    UPPER    = 'u',
    CAPITAL  = 'c',
    INVERT   = 'C',
    TOGGLE   = 't',
    TOGGLE_AT = 'T',
    APPEND   = '$',
    PREPEND  = '^',
    INSERT   = 'i',
    DELETE_  = 'D',
    OVERWRITE = 'o',
    TRUNCATE = '\'',
    REPLACE  = 's',
    PURGE    = '@',
    REVERSE  = 'r',
    DUPLICATE = 'd',
    DUPLICATE_N = 'p',
    REFLECT  = 'f',
    ROTATE_L = '{',
    ROTATE_R = '}',
    SWAP_FIRST = 'k',
    SWAP_LAST  = 'K',
    EXTRACT  = 'x',
    OMIT     = 'O',
    MEMORIZE = 'M',
    APPEND_MEM = '4',
    PREPEND_MEM = '6',
    REJECT_LEN_GT = '<',
    REJECT_LEN_LT = '>',
    REJECT_CHAR   = '!',
    REJECT_UNLESS = '/',
    INC_AT   = '+',
    DEC_AT   = '-',
};

inline uint32_t packOp(OpCode op, uint8_t p0 = 0, uint8_t p1 = 0) {
    return (static_cast<uint32_t>(op))
         | (static_cast<uint32_t>(p0) << 8)
         | (static_cast<uint32_t>(p1) << 16);
}

using CompiledRule = std::vector<uint32_t>;

}
}
