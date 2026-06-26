#pragma once

#include <string>
#include <cstdint>

namespace fenrir {
namespace ui {

enum class Color {
    Reset,
    Red, Green, Yellow, Blue, Magenta, Cyan, White,
    Bold, Dim
};

class Terminal {
public:
    /// Initialize terminal (detect platform, enable ANSI on Windows if needed)
    static void init();

    /// Clear entire screen and move cursor to (0,0)
    static void clearScreen();

    /// Move cursor to absolute position (row, col)
    static void moveCursor(int row, int col);

    /// Hide/show cursor
    static void hideCursor();
    static void showCursor();

    /// Get terminal width/height (returns 80x24 if unknown)
    static int width();
    static int height();

    /// Set console title bar text (Windows only, no-op on others)
    static void setTitle(const std::string& title);

    /// Write colored text at current cursor position
    static void write(const std::string& text, Color color = Color::Reset);

    /// Write a horizontal line of '─' characters
    static void writeHLine(int width = -1);

    /// Flush any buffered output
    static void flush();

    /// Restore terminal state (show cursor, reset colors)
    static void restore();

    /// Write raw ANSI escape code (for advanced control)
    static void writeAnsi(const std::string& code);

private:
    static bool s_initialized;
    static bool s_useAnsi;
};

} // namespace ui
} // namespace fenrir
