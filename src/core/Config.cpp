#include "Config.hpp"
#include "Constants.hpp"

#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>
#include <stdexcept>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <limits.h>
#endif

namespace fenrir {
namespace core {

using json = nlohmann::json;

static void mergeIf(json& target, const json& source, const char* key) {
    if (source.contains(key)) {
        target[key] = source[key];
    }
}

Config Config::load() {
    Config cfg;


    std::string defaultCfg = findDefaultConfig();
    if (!defaultCfg.empty()) {
        try {
            cfg.mergeFromJson(defaultCfg);
        } catch (const std::exception& e) {

        }
    }


    std::string userConfig;
#ifdef _WIN32
    char* appdata = std::getenv("APPDATA");
    if (appdata) {
        userConfig = std::string(appdata) + "\\.fenrir\\config.json";
    }
#else
    char* home = std::getenv("HOME");
    if (home) {
        userConfig = std::string(home) + "/.fenrir/config.json";
    }
#endif
    if (!userConfig.empty()) {
        try {
            cfg.mergeFromJson(userConfig);
        } catch (...) {}
    }


    try {
        cfg.mergeFromJson("fenrir.json");
    } catch (...) {}


    cfg.mergeFromEnv();

    return cfg;
}

void Config::mergeFromJson(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    json j;
    f >> j;


    if (j.contains("batch")) {
        auto& b = j["batch"];
        if (b.contains("fast_hash_size")) fastHashBatchSize = b["fast_hash_size"];
        if (b.contains("slow_hash_size")) slowHashBatchSize = b["slow_hash_size"];
        if (b.contains("api_size"))       apiBatchSize      = b["api_size"];
    }


    if (j.contains("checkpoint")) {
        auto& c = j["checkpoint"];
        if (c.contains("interval_seconds")) checkpointIntervalSeconds = c["interval_seconds"];
        if (c.contains("file"))             checkpointFile            = c["file"];
    }


    if (j.contains("gpu")) {
        auto& g = j["gpu"];
        if (g.contains("default_device"))        gpuDevice         = g["default_device"];
        if (g.contains("work_group_auto_tune"))  workGroupAutoTune = g["work_group_auto_tune"];
        if (g.contains("kernel_cache_dir"))      kernelCacheDir    = g["kernel_cache_dir"];
        if (g.contains("async_pipeline"))        asyncPipeline     = g["async_pipeline"];
        if (g.contains("async_buffers"))          asyncBuffers      = g["async_buffers"];
        if (g.contains("cpu_only"))             cpuOnly            = g["cpu_only"];
    }

    if (j.contains("cpu")) {
        auto& cp = j["cpu"];
        if (cp.contains("enable_simd"))           enableSIMD        = cp["enable_simd"];
        if (cp.contains("cpu_only"))             cpuOnly            = cp["cpu_only"];
    }


    if (j.contains("api")) {
        auto& a = j["api"];
        if (a.contains("timeout_seconds"))    apiTimeoutSeconds = a["timeout_seconds"];
        if (a.contains("max_retries"))        apiMaxRetries     = a["max_retries"];
        if (a.contains("rate_limit_buffer"))   apiRateLimitBuf   = a["rate_limit_buffer"];
        if (a.contains("api_key"))            apiKey            = a["api_key"];
        if (a.contains("provider"))           apiProvider       = a["provider"];
    }


    if (j.contains("logging")) {
        auto& l = j["logging"];
        if (l.contains("level"))           logLevel      = l["level"];
        if (l.contains("console_colors"))  consoleColors = l["console_colors"];
        if (l.contains("file"))            logFile       = l["file"];
    }


    if (j.contains("output")) {
        auto& o = j["output"];
        if (o.contains("cracked_file")) outputFile   = o["cracked_file"];
        if (o.contains("append_mode"))  outputAppend = o["append_mode"];
        if (o.contains("potfile"))      potfile      = o["potfile"];
    }

    if (j.contains("benchmark")) {
        auto& bm = j["benchmark"];
        if (bm.contains("enable")) benchmark = bm["enable"];
        if (bm.contains("count"))  benchmarkCount = bm["count"];
    }

    if (j.contains("attack")) {
        auto& atk = j["attack"];
        if (atk.contains("wordlist2")) wordlist2 = atk["wordlist2"];
    }
}

void Config::mergeFromEnv() {
    auto getEnv = [](const char* name) -> const char* {
        return std::getenv(name);
    };

    if (auto* v = getEnv("FENRIR_BATCH_SIZE"))     fastHashBatchSize = std::stoull(v);
    if (auto* v = getEnv("FENRIR_GPU_DEVICE"))     gpuDevice         = std::stoi(v);
    if (auto* v = getEnv("FENRIR_CPU_ONLY"))       cpuOnly           = (std::string(v) == "1" || std::string(v) == "true");
    if (auto* v = getEnv("FENRIR_API_KEY"))        apiKey            = v;
    if (auto* v = getEnv("FENRIR_API_PROVIDER"))   apiProvider       = v;
    if (auto* v = getEnv("FENRIR_API_TIMEOUT"))     apiTimeoutSeconds = std::stoi(v);
    if (auto* v = getEnv("FENRIR_LOG_LEVEL"))      logLevel          = v;
    if (auto* v = getEnv("FENRIR_LOG_FILE"))       logFile           = v;
    if (auto* v = getEnv("FENRIR_OUTPUT_FILE"))    outputFile        = v;
    if (auto* v = getEnv("FENRIR_POTFILE"))       potfile           = v;
}

std::string Config::findDefaultConfig() {

    std::ifstream test("config/default_config.json");
    if (test.is_open()) return "config/default_config.json";


#ifdef _WIN32
    char exePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    if (len > 0) {
        std::string dir(exePath, len);
        auto pos = dir.find_last_of("\\/");
        if (pos != std::string::npos) {
            dir = dir.substr(0, pos);
            std::string cfg = dir + "\\..\\share\\fenrir\\config\\default_config.json";
            std::ifstream test2(cfg);
            if (test2.is_open()) return cfg;
        }
    }
#else
    char exePath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len > 0) {
        exePath[len] = '\0';
        std::string dir(exePath);
        auto pos = dir.find_last_of('/');
        if (pos != std::string::npos) {
            dir = dir.substr(0, pos);
            std::string cfg = dir + "/../share/fenrir/config/default_config.json";
            std::ifstream test2(cfg);
            if (test2.is_open()) return cfg;
        }
    }
#endif

    return "";
}

}
}
