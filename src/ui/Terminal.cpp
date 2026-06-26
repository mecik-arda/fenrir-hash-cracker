#include "Terminal.hpp"

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  #include <io.h>
#else
  #include <sys/ioctl.h>
  #include <unistd.h>
#endif

#include <iostream>
#include <cstdio>

namespace fenrir {
namespace ui {

bool Terminal::s_initialized = false;
bool Terminal::s_useAnsi = false;

void Terminal::init() {
    if (s_initialized) return;
    s_initialized = true;

#ifdef _WIN32
    // Enable ANSI escape code processing on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
#endif
    s_useAnsi = true; // All modern terminals support ANSI
}

void Terminal::writeAnsi(const std::string& code) {
    std::cout << "\033[" << code;
}

void Terminal::clearScreen() {
    if (s_useAnsi) {
        writeAnsi("2J");
        writeAnsi("H");
    }
}

void Terminal::moveCursor(int row, int col) {
    if (s_useAnsi) {
        writeAnsi(std::to_string(row) + ";" + std::to_string(col) + "H");
    }
}

void Terminal::hideCursor() {
    if (s_useAnsi) writeAnsi("?25l");
}

void Terminal::showCursor() {
    if (s_useAnsi) writeAnsi("?25h");
}

int Terminal::width() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) return w.ws_col;
#endif
    return 80;
}

int Terminal::height() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) return w.ws_row;
#endif
    return 24;
}

void Terminal::setTitle(const std::string& title) {
#ifdef _WIN32
    SetConsoleTitleA(title.c_str());
#else
    if (s_useAnsi) writeAnsi("]0;" + title + "\007");
#endif
}

void Terminal::write(const std::string& text, Color color) {
    if (s_useAnsi) {
        switch (color) {
            case Color::Red:     writeAnsi("31m"); break;
            case Color::Green:   writeAnsi("32m"); break;
            case Color::Yellow:  writeAnsi("33m"); break;
            case Color::Blue:    writeAnsi("34m"); break;
            case Color::Magenta: writeAnsi("35m"); break;
            case Color::Cyan:    writeAnsi("36m"); break;
            case Color::White:   writeAnsi("37m"); break;
            case Color::Bold:    writeAnsi("1m"); break;
            case Color::Dim:     writeAnsi("2m"); break;
            default: break;
        }
    }
    std::cout << text;
    if (s_useAnsi && color != Color::Reset) writeAnsi("0m");
}

void Terminal::writeHLine(int width) {
    int w = (width < 0) ? Terminal::width() : width;
    std::cout << std::string(w, '-');
}

void Terminal::flush() {
    std::cout << std::flush;
}

void Terminal::restore() {
    showCursor();
    if (s_useAnsi) writeAnsi("0m");
    std::cout << std::flush;
}

} // namespace ui
} // namespace fenrir
