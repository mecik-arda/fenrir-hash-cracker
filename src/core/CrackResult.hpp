#pragma once

#include "TargetHash.hpp"
#include <string>
#include <cstdint>

namespace fenrir {
namespace core {

struct CrackResult {
    std::string hash;
    std::string plaintext;
    HashType    algorithm;
    double      secondsFromStart = 0.0;
    uint64_t    attemptNumber = 0;

    CrackResult() = default;
    CrackResult(std::string h, std::string pw, HashType algo,
                double secs = 0.0, uint64_t attempt = 0)
        : hash(std::move(h)), plaintext(std::move(pw)),
          algorithm(algo), secondsFromStart(secs), attemptNumber(attempt) {}
};

}
}
