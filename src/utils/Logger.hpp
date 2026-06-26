#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <string>

namespace fenrir {
namespace utils {

class Logger {
public:

    static void init(const std::string& level = "info",
                     const std::string& logFile = "",
                     bool consoleColors = true);


    static std::shared_ptr<spdlog::logger> get() {
        return spdlog::get("fenrir");
    }


    static void trace(const std::string& msg) { auto l = get(); if (l) l->trace(msg); }
    static void debug(const std::string& msg) { auto l = get(); if (l) l->debug(msg); }
    static void info(const std::string& msg)  { auto l = get(); if (l) l->info(msg); }
    static void warn(const std::string& msg)  { auto l = get(); if (l) l->warn(msg); }
    static void error(const std::string& msg) { auto l = get(); if (l) l->error(msg); }
    static void critical(const std::string& msg) { auto l = get(); if (l) l->critical(msg); }
};

}
}
