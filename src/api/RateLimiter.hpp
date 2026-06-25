#pragma once

#include <chrono>

namespace fenrir {
namespace api {

class RateLimiter {
public:


    explicit RateLimiter(double maxPerSecond, double burstFactor = 1.1);



    void wait();



    bool tryConsume();


    void setRate(double maxPerSecond);

private:
    double m_rate;
    double m_burstCapacity;
    double m_tokens;
    std::chrono::steady_clock::time_point m_lastRefill;
};

}
}
