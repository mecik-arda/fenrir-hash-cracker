#pragma once

#include <functional>
#include <atomic>

namespace fenrir {
namespace utils {

class SignalHandler {
public:
    using Callback = std::function<void()>;


    static void install(Callback onInterrupt);


    static bool interrupted();


    static void reset();

private:
    static std::atomic<bool> s_interrupted;
    static Callback s_callback;
};

}
}
