#pragma once

#include <string>
#include <cstdint>

namespace fenrir {
namespace core {

struct Candidate {
    std::string plaintext;
    uint32_t    sourceIndex = 0;

    Candidate() = default;
    Candidate(std::string pw, uint32_t idx = 0)
        : plaintext(std::move(pw)), sourceIndex(idx) {}
};

}
}
