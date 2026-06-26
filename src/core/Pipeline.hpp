#pragma once

#include "Config.hpp"
#include "TargetHash.hpp"
#include "IHashEngine.hpp"
#include <vector>
#include <string>
#include <memory>

namespace fenrir {
namespace core {

class Pipeline {
public:
    Pipeline(const Config& config);



    int run();


    uint64_t attempts() const;
    uint64_t cracked() const;
    double   hashRate() const;
    double   etaSeconds() const;

private:
    int runApiLookup();

    Config m_config;
    std::unique_ptr<IHashEngine> m_engine;
    std::vector<TargetHash> m_targets;
    uint64_t m_attempts = 0;
    uint64_t m_cracked = 0;
};

}
}
