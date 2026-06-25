#include "Logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace fenrir {
namespace utils {

void Logger::init(const std::string& level,
                   const std::string& logFile,
                   bool consoleColors) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    if (consoleColors) {
        console_sink->set_pattern("[%^%l%$] %H:%M:%S %v");
    } else {
        console_sink->set_pattern("[%l] %H:%M:%S %v");
    }

    std::vector<spdlog::sink_ptr> sinks{console_sink};

    if (!logFile.empty()) {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
        sinks.push_back(file_sink);
    }

    auto logger = std::make_shared<spdlog::logger>("fenrir", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::from_str(level));
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
}

}
}
