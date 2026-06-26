#pragma once

#include <string>
#include <vector>
#include <cstddef>

namespace fenrir {
namespace core {

struct Config {

    std::size_t fastHashBatchSize = 1'000'000;
    std::size_t slowHashBatchSize = 1'024;
    std::size_t apiBatchSize      = 100;


    int         checkpointIntervalSeconds = 30;
    std::string checkpointFile            = "fenrir.checkpoint";


    int         gpuDevice          = 0;
    bool        workGroupAutoTune  = true;
    std::string kernelCacheDir     = ".fenrir_kernel_cache";
    bool        cpuOnly            = false;
    bool        enableSIMD         = true;
    bool        asyncPipeline      = true;
    int         asyncBuffers       = 2;


    int         apiTimeoutSeconds = 10;
    int         apiMaxRetries     = 3;
    double      apiRateLimitBuf   = 1.1;
    std::string apiKey;
    std::string apiProvider;
    bool        apiFallback = true;


    std::string logLevel       = "info";
    bool        consoleColors  = true;
    std::string logFile;


    std::string outputFile   = "cracked.txt";
    bool        outputAppend = false;
    std::string potfile      = "fenrir.pot";


    std::string attack;
    std::string hashMode;
    std::string wordlist;
    std::string wordlist2;
    std::string ruleFile;
    std::string maskPattern;
    int         minLength = 1;
    int         maxLength = 8;
    std::string charset;


    std::string customCharset1;
    std::string customCharset2;
    std::string customCharset3;
    std::string customCharset4;


    std::vector<std::string> hashFiles;
    std::vector<std::string> inlineHashes;


    bool   resume     = false;
    bool   showVersion = false;
    bool   showHelp    = false;
    bool   benchmark   = false;
    uint64_t benchmarkCount = 100'000;
    bool   noTui       = false;
    std::string configFile;



    static Config load();



    void mergeFromJson(const std::string& path);


    void mergeFromEnv();

private:

    static std::string findDefaultConfig();
};

}
}
