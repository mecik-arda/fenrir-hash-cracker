#include "ProgressDisplay.hpp"
#include "Terminal.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace fenrir {
namespace ui {

ProgressDisplay::ProgressDisplay(const std::string& hashMode, const std::string& attackMode,
                                 const std::string& engineInfo, uint64_t totalEstimate)
    : m_hashMode(hashMode), m_attackMode(attackMode),
      m_engineInfo(engineInfo), m_totalEstimate(totalEstimate)
{
    Terminal::init();
    Terminal::clearScreen();
    Terminal::hideCursor();
    Terminal::setTitle("Fenrir Hash Cracker - " + hashMode + " / " + attackMode);
    render();
}

ProgressDisplay::~ProgressDisplay() {
    if (m_enabled) {
        Terminal::restore();
    }
}

void ProgressDisplay::update(uint64_t tested, uint64_t cracked, uint64_t remaining,
                              const std::string& hashRate, const std::string& eta,
                              const std::string& elapsed) {
    if (!m_enabled) return;
    m_tested = tested;
    m_cracked = cracked;
    m_remaining = remaining;
    m_hashRate = hashRate;
    m_eta = eta;
    m_elapsed = elapsed;
    render();
}

void ProgressDisplay::onCrack(const std::string& hashOrUser, const std::string& password) {
    if (!m_enabled) return;
    // Briefly show cracked notification at the bottom
    Terminal::moveCursor(Terminal::height() - 1, 0);
    Terminal::write("[CRACKED] ", Color::Green);
    Terminal::write(hashOrUser + " → " + password, Color::Bold);
    Terminal::writeAnsi("K"); // clear to end of line
}

void ProgressDisplay::render() {
    renderHeader();
    renderProgress();
    renderStats();
    renderFooter();
    Terminal::flush();
}

void ProgressDisplay::renderHeader() {
    Terminal::moveCursor(0, 0);
    Terminal::write("+", Color::Dim);
    Terminal::writeHLine(Terminal::width() - 2);
    Terminal::write("+", Color::Dim);
    Terminal::moveCursor(1, 0);
    Terminal::write("| ", Color::Dim);
    Terminal::write("🐺 Fenrir " + m_hashMode + " · " + m_attackMode, Color::Bold);
    std::string info = " · " + m_engineInfo;
    Terminal::write(info, Color::Cyan);
    Terminal::write(std::string(std::max(0, Terminal::width() - (int)info.size() - 30), ' '));
    Terminal::write(" |", Color::Dim);

    Terminal::moveCursor(2, 0);
    Terminal::write("+", Color::Dim);
    Terminal::writeHLine(Terminal::width() - 2);
    Terminal::write("+", Color::Dim);
}

void ProgressDisplay::renderProgress() {
    if (m_totalEstimate == 0) return;
    Terminal::moveCursor(3, 0);
    Terminal::write("| ", Color::Dim);

    double pct = std::min(100.0, (m_totalEstimate > 0)
        ? (100.0 * static_cast<double>(m_tested) / static_cast<double>(m_totalEstimate)) : 0.0);

    int barW = Terminal::width() - 12;
    int filled = static_cast<int>(barW * pct / 100.0);

    std::ostringstream bar;
    bar << "Progress: " << std::fixed << std::setprecision(1) << pct << "% ";
    Terminal::write(bar.str(), Color::White);

    Terminal::write(std::string(filled, '#'), Color::Green);
    Terminal::write(std::string(barW - filled, '.'), Color::Dim);

    Terminal::write(" |", Color::Dim);
}

void ProgressDisplay::renderStats() {
    int row = 4;
    Terminal::moveCursor(row++, 0);
    Terminal::write("| ", Color::Dim);
    Terminal::write("Tested: ", Color::White);
    Terminal::write(std::to_string(m_tested), Color::Bold);
    Terminal::write(" | Cracked: ", Color::White);
    Terminal::write(std::to_string(m_cracked), Color::Green);
    Terminal::write(" | Remaining: ", Color::White);
    Terminal::write(std::to_string(m_remaining), Color::Yellow);
    Terminal::write(std::string(std::max(0, Terminal::width() - 55), ' '));
    Terminal::write(" |", Color::Dim);

    Terminal::moveCursor(row++, 0);
    Terminal::write("+", Color::Dim);
    Terminal::writeHLine(Terminal::width() - 2);
    Terminal::write("+", Color::Dim);
}

void ProgressDisplay::renderFooter() {
    int row = 7;
    Terminal::moveCursor(row, 0);
    Terminal::write("| ", Color::Dim);
    Terminal::write("Speed: " + m_hashRate + " | ETA: " + m_eta + " | Elapsed: " + m_elapsed, Color::White);
    Terminal::write(std::string(std::max(0, Terminal::width() - 45), ' '));
    Terminal::write(" |", Color::Dim);

    Terminal::moveCursor(row + 1, 0);
    Terminal::write("+", Color::Dim);
    Terminal::writeHLine(Terminal::width() - 2);
    Terminal::write("+", Color::Dim);
}

void ProgressDisplay::summary(uint64_t tested, uint64_t cracked,
                               const std::string& elapsed, const std::string& speed) {
    Terminal::restore();
    Terminal::write("\n");
    Terminal::write("  Time:    " + elapsed + "\n", Color::White);
    Terminal::write("  Tested:  " + std::to_string(tested) + "\n", Color::White);
    Terminal::write("  Cracked: " + std::to_string(cracked) + "\n", Color::Green);
    Terminal::write("  Speed:   " + speed + "\n", Color::Bold);
    m_enabled = false;
}

} // namespace ui
} // namespace fenrir
