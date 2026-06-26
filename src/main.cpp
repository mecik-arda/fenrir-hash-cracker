#include "core/Config.hpp"
#include "core/Constants.hpp"
#include "core/Pipeline.hpp"
#include "utils/Logger.hpp"
#include "utils/Platform.hpp"

#include <CLI/CLI.hpp>
#include <iostream>

int main(int argc, char** argv) {
    using namespace fenrir;

    CLI::App app{core::DESCRIPTION};
    app.set_version_flag("--version,-V", core::VERSION_STRING);

    core::Config cliConfig;

    app.add_option("-m,--mode", cliConfig.hashMode,
        "Hash mode: md5, sha1, sha256, sha512, ntlm, bcrypt, scrypt");

    app.add_option("-a,--attack", cliConfig.attack,
        "Attack mode: dict, rule, mask, hybrid, api");

    app.add_option("-H,--hashes", cliConfig.hashFiles,
        "Target hash file(s)");

    app.add_option("--hash", cliConfig.inlineHashes,
        "Inline target hash(es)");

    app.add_option("-w,--wordlist", cliConfig.wordlist,
        "Wordlist file path");

    app.add_option("-r,--rule-file", cliConfig.ruleFile,
        "Rule file (.rule) path");

    app.add_option("-p,--mask-pattern", cliConfig.maskPattern,
        "Mask pattern (e.g., ?l?l?l?d?d)");

    app.add_option("--min-len", cliConfig.minLength,
        "Minimum password length")->default_val(1);

    app.add_option("--max-len", cliConfig.maxLength,
        "Maximum password length")->default_val(8);

    app.add_option("--charset", cliConfig.charset,
        "Character set for brute-force");

    app.add_option("-1", cliConfig.customCharset1,
        "Custom charset 1 (?1)");
    app.add_option("-2", cliConfig.customCharset2,
        "Custom charset 2 (?2)");
    app.add_option("-3", cliConfig.customCharset3,
        "Custom charset 3 (?3)");
    app.add_option("-4", cliConfig.customCharset4,
        "Custom charset 4 (?4)");


    app.add_option("--provider", cliConfig.apiProvider,
        "API provider: leaklookup, hashkiller");

    app.add_option("--api-key", cliConfig.apiKey,
        "API key (or set FENRIR_API_KEY env var)");

    app.add_flag("--no-api-fallback{false}", cliConfig.apiFallback,
        "Disable API fallback when offline exhausted");


    app.add_flag("--cpu-only", cliConfig.cpuOnly,
        "Force CPU-only mode (no GPU)");

    app.add_option("-d,--device", cliConfig.gpuDevice,
        "GPU device index")->default_val(0);

    app.add_option("--batch-size", cliConfig.fastHashBatchSize,
        "Candidate batch size (0 = auto)");

    app.add_option("--checkpoint-interval", cliConfig.checkpointIntervalSeconds,
        "Checkpoint save interval in seconds")->default_val(30);

    app.add_flag("--no-simd{false}", cliConfig.enableSIMD,
        "Disable SIMD (AVX2/AVX-512) CPU acceleration");

    app.add_flag("--no-async{false}", cliConfig.asyncPipeline,
        "Disable async GPU double-buffering");

    app.add_option("--async-buffers", cliConfig.asyncBuffers,
        "Number of async GPU buffers (2-4)")->default_val(2);


    app.add_flag("--resume", cliConfig.resume,
        "Resume from checkpoint");

    app.add_option("-c,--config", cliConfig.configFile,
        "Config file path");

    app.add_option("-o,--output", cliConfig.outputFile,
        "Output file for cracked passwords")->default_str("cracked.txt");

    app.add_option("--log-file", cliConfig.logFile,
        "Log file path");

    app.add_option("--log-level", cliConfig.logLevel,
        "Log level: trace, debug, info, warn, error")->default_val("info");


    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }


    auto config = core::Config::load();

    if (app.count("--mode"))          config.hashMode     = cliConfig.hashMode;
    if (app.count("--attack"))        config.attack       = cliConfig.attack;
    if (app.count("--hashes"))        config.hashFiles    = cliConfig.hashFiles;
    if (app.count("--hash"))          config.inlineHashes = cliConfig.inlineHashes;
    if (app.count("--wordlist"))      config.wordlist     = cliConfig.wordlist;
    if (app.count("--rule-file"))     config.ruleFile     = cliConfig.ruleFile;
    if (app.count("--mask-pattern"))  config.maskPattern  = cliConfig.maskPattern;
    if (app.count("--min-len"))       config.minLength    = cliConfig.minLength;
    if (app.count("--max-len"))       config.maxLength    = cliConfig.maxLength;
    if (app.count("--charset"))       config.charset      = cliConfig.charset;
    if (app.count("-1"))              config.customCharset1 = cliConfig.customCharset1;
    if (app.count("-2"))              config.customCharset2 = cliConfig.customCharset2;
    if (app.count("-3"))              config.customCharset3 = cliConfig.customCharset3;
    if (app.count("-4"))              config.customCharset4 = cliConfig.customCharset4;
    if (app.count("--provider"))      config.apiProvider  = cliConfig.apiProvider;
    if (app.count("--api-key"))       config.apiKey       = cliConfig.apiKey;
    if (app.count("--no-api-fallback")) config.apiFallback = cliConfig.apiFallback;
    if (app.count("--cpu-only"))      config.cpuOnly      = cliConfig.cpuOnly;
    if (app.count("--device"))        config.gpuDevice     = cliConfig.gpuDevice;
    if (app.count("--batch-size"))    config.fastHashBatchSize = cliConfig.fastHashBatchSize;
    if (app.count("--checkpoint-interval")) config.checkpointIntervalSeconds = cliConfig.checkpointIntervalSeconds;
    if (app.count("--no-simd"))       config.enableSIMD    = cliConfig.enableSIMD;
    if (app.count("--no-async"))      config.asyncPipeline  = cliConfig.asyncPipeline;
    if (app.count("--async-buffers")) config.asyncBuffers   = cliConfig.asyncBuffers;
    if (app.count("--resume"))        config.resume       = cliConfig.resume;
    if (app.count("--config"))        config.configFile   = cliConfig.configFile;
    if (app.count("--output"))        config.outputFile   = cliConfig.outputFile;
    if (app.count("--log-file"))      config.logFile      = cliConfig.logFile;
    if (app.count("--log-level"))     config.logLevel     = cliConfig.logLevel;


    if (!config.configFile.empty()) {
        try {
            config.mergeFromJson(config.configFile);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Cannot load config file '" << config.configFile
                      << "': " << e.what() << std::endl;
        }
    }


    utils::Logger::init(config.logLevel, config.logFile, config.consoleColors);


    utils::Logger::info(core::VERSION_STRING);
    utils::Logger::info("Platform: " + utils::platformName() +
                        " | CPU cores: " + std::to_string(utils::cpuCoreCount()));
    utils::Logger::info("================================================");


    std::vector<std::string> errors;

    if (config.hashMode.empty()) {
        errors.push_back("Hash mode (-m/--mode) is required");
    }
    if (config.attack.empty()) {
        errors.push_back("Attack mode (-a/--attack) is required");
    }
    if (config.hashFiles.empty() && config.inlineHashes.empty()) {
        errors.push_back("Target hash file (-H) or inline hash (--hash) is required");
    }


    if (config.attack == "dict" || config.attack == "rule") {
        if (config.wordlist.empty()) {
            errors.push_back("Wordlist (-w) is required for dict/rule attacks");
        }
    }
    if (config.attack == "rule" && config.ruleFile.empty()) {
        errors.push_back("Rule file (-r) is required for rule-based attack");
    }
    if (config.attack == "mask" && config.maskPattern.empty() &&
        config.charset.empty()) {
        errors.push_back("Mask pattern (-p) or charset (--charset) is required for mask attack");
    }
    if (config.attack == "api" && config.apiProvider.empty()) {
        errors.push_back("API provider (--provider) is required for API attack");
    }

    if (!errors.empty()) {
        for (const auto& err : errors) {
            utils::Logger::error(err);
        }
        utils::Logger::info("Use --help for usage information");
        return 1;
    }


    utils::Logger::info("Configuration:");
    utils::Logger::info("  Hash mode:    " + config.hashMode);
    utils::Logger::info("  Attack mode:  " + config.attack);
    if (!config.hashFiles.empty()) {
        for (const auto& f : config.hashFiles) {
            utils::Logger::info("  Hash file:    " + f);
        }
    }
    if (!config.inlineHashes.empty()) {
        utils::Logger::info("  Inline hashes: " + std::to_string(config.inlineHashes.size()));
    }
    if (!config.wordlist.empty())
        utils::Logger::info("  Wordlist:     " + config.wordlist);
    if (!config.ruleFile.empty())
        utils::Logger::info("  Rule file:    " + config.ruleFile);
    if (!config.maskPattern.empty())
        utils::Logger::info("  Mask pattern: " + config.maskPattern);
    if (!config.apiProvider.empty())
        utils::Logger::info("  API provider: " + config.apiProvider);
    utils::Logger::info("  Output file:  " + config.outputFile);
    utils::Logger::info("  GPU mode:     " + std::string(config.cpuOnly ? "CPU only" : "GPU enabled"));
    utils::Logger::info("================================================");


    try {
        core::Pipeline pipeline(config);
        int cracked = pipeline.run();
        if (cracked > 0) {
            utils::Logger::info("Successfully cracked " + std::to_string(cracked) + " hash(es)!");
        } else {
            utils::Logger::info("No hashes were cracked.");
        }
        return cracked > 0 ? 0 : 1;
    } catch (const std::exception& e) {
        utils::Logger::error(std::string("Pipeline error: ") + e.what());
        return 2;
    }
}
