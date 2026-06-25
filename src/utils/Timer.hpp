#pragma once

#include <chrono>
#include <string>

namespace fenrir {
namespace utils {

class Timer {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    Timer() : m_start(Clock::now()), m_running(true) {}


    void start() {
        m_start = Clock::now();
        m_running = true;
    }


    void pause() {
        if (m_running) {
            m_accumulated += Clock::now() - m_start;
            m_running = false;
        }
    }


    void resume() {
        if (!m_running) {
            m_start = Clock::now();
            m_running = true;
        }
    }


    void reset() {
        m_start = Clock::now();
        m_accumulated = Duration::zero();
        m_running = true;
    }


    double elapsedSeconds() const {
        return std::chrono::duration<double>(elapsed()).count();
    }


    double elapsedMillis() const {
        return std::chrono::duration<double, std::milli>(elapsed()).count();
    }


    Duration elapsed() const {
        auto d = m_accumulated;
        if (m_running) {
            d += Clock::now() - m_start;
        }
        return d;
    }


    std::string elapsedFormatted() const {
        auto secs = static_cast<int64_t>(elapsedSeconds());
        auto hours   = secs / 3600;
        auto minutes = (secs % 3600) / 60;
        auto seconds = secs % 60;

        if (hours > 0) {
            return std::to_string(hours) + "h " +
                   std::to_string(minutes) + "m " +
                   std::to_string(seconds) + "s";
        } else if (minutes > 0) {
            return std::to_string(minutes) + "m " +
                   std::to_string(seconds) + "s";
        } else {
            return std::to_string(seconds) + "s";
        }
    }

private:
    TimePoint m_start;
    Duration  m_accumulated{0};
    bool      m_running;
};

}
}
