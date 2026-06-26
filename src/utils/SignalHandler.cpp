#include "SignalHandler.hpp"

#include <csignal>

namespace fenrir {
namespace utils {

std::atomic<bool> SignalHandler::s_interrupted{false};
SignalHandler::Callback SignalHandler::s_callback;

static void signalHandler(int) {
    SignalHandler::s_interrupted.store(true);
    if (SignalHandler::s_callback) {
        SignalHandler::s_callback();
    }
}

void SignalHandler::install(Callback onInterrupt) {
    s_callback = std::move(onInterrupt);
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
}

bool SignalHandler::interrupted() {
    return s_interrupted.load();
}

void SignalHandler::reset() {
    s_interrupted.store(false);
}

}
}
