#pragma once

#include <cstdint>
#include <string>

namespace fenrir {
namespace gpu {

class GpuStats {
public:
    void addAttempts(uint64_t count);
    void updateTimer(double elapsedSeconds);


    double hashRate() const;


    double etaSeconds(uint64_t remaining) const;


    std::string hashRateFormatted() const;


    std::string etaFormatted(uint64_t remaining) const;


    void reset();

private:
    uint64_t m_totalAttempts = 0;
    double   m_elapsedSeconds = 0.0;

    static std::string formatNumber(double value, const char* suffix);
};

}
}
