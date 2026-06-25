#include "GpuStats.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace fenrir {
namespace gpu {

void GpuStats::addAttempts(uint64_t count) {
    m_totalAttempts += count;
}

void GpuStats::updateTimer(double elapsedSeconds) {
    m_elapsedSeconds = elapsedSeconds;
}

double GpuStats::hashRate() const {
    if (m_elapsedSeconds <= 0.0) return 0.0;
    return static_cast<double>(m_totalAttempts) / m_elapsedSeconds;
}

double GpuStats::etaSeconds(uint64_t remaining) const {
    double rate = hashRate();
    if (rate <= 0.0) return -1.0;
    return static_cast<double>(remaining) / rate;
}

std::string GpuStats::hashRateFormatted() const {
    return formatNumber(hashRate(), "H/s");
}

std::string GpuStats::etaFormatted(uint64_t remaining) const {
    double secs = etaSeconds(remaining);
    if (secs < 0) return "Unknown";

    auto totalSecs = static_cast<int64_t>(secs);
    auto hours   = totalSecs / 3600;
    auto minutes = (totalSecs % 3600) / 60;
    auto seconds = totalSecs % 60;

    std::ostringstream oss;
    if (hours > 0) {
        oss << hours << "h " << minutes << "m " << seconds << "s";
    } else if (minutes > 0) {
        oss << minutes << "m " << seconds << "s";
    } else {
        oss << seconds << "s";
    }
    return oss.str();
}

void GpuStats::reset() {
    m_totalAttempts = 0;
    m_elapsedSeconds = 0.0;
}

std::string GpuStats::formatNumber(double value, const char* suffix) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);

    if (value >= 1e12) {
        oss << (value / 1e12) << " T" << suffix;
    } else if (value >= 1e9) {
        oss << (value / 1e9) << " G" << suffix;
    } else if (value >= 1e6) {
        oss << (value / 1e6) << " M" << suffix;
    } else if (value >= 1e3) {
        oss << (value / 1e3) << " k" << suffix;
    } else {
        oss << static_cast<int64_t>(value) << " " << suffix;
    }
    return oss.str();
}

}
}
