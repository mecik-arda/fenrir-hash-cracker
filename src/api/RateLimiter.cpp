#include "RateLimiter.hpp"

#include <thread>

namespace fenrir {
namespace api {

RateLimiter::RateLimiter(double maxPerSecond, double burstFactor)
    : m_rate(maxPerSecond)
    , m_burstCapacity(maxPerSecond * burstFactor)
    , m_tokens(maxPerSecond * burstFactor)
    , m_lastRefill(std::chrono::steady_clock::now())
{
}

void RateLimiter::wait() {
    while (!tryConsume()) {

        double interval = 1.0 / m_rate;
        std::this_thread::sleep_for(
            std::chrono::microseconds(static_cast<int64_t>(interval * 1'000'000 * 0.5)));
    }
}

bool RateLimiter::tryConsume() {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - m_lastRefill).count();


    m_tokens += elapsed * m_rate;
    if (m_tokens > m_burstCapacity) {
        m_tokens = m_burstCapacity;
    }
    m_lastRefill = now;


    if (m_tokens >= 1.0) {
        m_tokens -= 1.0;
        return true;
    }
    return false;
}

void RateLimiter::setRate(double maxPerSecond) {
    m_rate = maxPerSecond;
    m_burstCapacity = maxPerSecond * 1.1;
}

}
}
