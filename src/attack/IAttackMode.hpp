#pragma once

#include "../core/Config.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>

namespace fenrir {
namespace attack {

class IAttackMode {
public:
    virtual ~IAttackMode() = default;


    virtual std::string name() const = 0;


    virtual void initialize(const core::Config& cfg) = 0;


    virtual bool nextBatch(std::vector<std::string>& candidates,
                           size_t batchSize) = 0;


    virtual std::vector<uint8_t> serializeState() const = 0;


    virtual void deserializeState(const std::vector<uint8_t>& data) = 0;



    virtual uint64_t totalCandidateEstimate() const = 0;


    virtual uint64_t candidatesGenerated() const = 0;


    virtual bool isExhausted() const = 0;
};

}
}
