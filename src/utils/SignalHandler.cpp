#include "SignalHandler.hpp"

#include <csignal>

namespace fenrir {
namespace utils {

std::atomic<bool> SignalHandler::s_interrupted{false};
SignalHandler::Callback SignalHandler::s_callback;

static void signalHandler(int) {
    // Only set atomic flag in signal handler (signal-safe).
    // The callback is polled by the main loop during safe points.
    SignalHandler::s_interrupted.store(true);
}

void SignalHandler::install(Callback onInterrupt) {
    s_callback = std::move(onInterrupt);
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
}

bool SignalHandler::interrupted() {
    return s_interrupted.load();
}

void SignalHandler::checkAndInvoke() {
    if (s_interrupted.load() && s_callback) {
        s_callback();
        s_interrupted.store(false);
    }
}

void SignalHandler::reset() {
    s_interrupted.store(false);
}

}
}
