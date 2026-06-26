#pragma once

#include <string>
#include <cstdint>

namespace fenrir {
namespace ui {

class ProgressDisplay {
public:
    ProgressDisplay(const std::string& hashMode, const std::string& attackMode,
                    const std::string& engineInfo, uint64_t totalEstimate);

    ~ProgressDisplay();

    /// Update stats (call from main loop)
    void update(uint64_t tested, uint64_t cracked, uint64_t remaining,
                const std::string& hashRate, const std::string& eta,
                const std::string& elapsed);

    /// Mark a hash as cracked (shows username if available)
    void onCrack(const std::string& hashOrUser, const std::string& password);

    /// Final summary after attack completes
    void summary(uint64_t tested, uint64_t cracked, const std::string& elapsed,
                 const std::string& speed);

private:
    void render();
    void renderHeader();
    void renderProgress();
    void renderStats();
    void renderFooter();

    std::string m_hashMode;
    std::string m_attackMode;
    std::string m_engineInfo;
    uint64_t m_totalEstimate;

    uint64_t m_tested = 0;
    uint64_t m_cracked = 0;
    uint64_t m_remaining = 0;
    std::string m_hashRate;
    std::string m_eta;
    std::string m_elapsed;
    bool m_enabled = true;
};

} // namespace ui
} // namespace fenrir
