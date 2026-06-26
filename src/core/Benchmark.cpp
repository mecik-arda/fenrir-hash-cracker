#include "Benchmark.hpp"
#include "HashEngineFactory.hpp"
#ifdef FENRIR_HAS_GPU
#include "../gpu/OpenCLContext.hpp"
#include "../gpu/GpuHasher.hpp"
#endif
#include "../utils/Logger.hpp"
#include "../utils/Timer.hpp"
#include "../cpu_simd/SIMDDetector.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace fenrir {
namespace core {

static std::string formatHps(double hps) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    if (hps >= 1e9)       ss << (hps / 1e9) << " GH/s";
    else if (hps >= 1e6)  ss << (hps / 1e6) << " MH/s";
    else if (hps >= 1e3)  ss << (hps / 1e3) << " kH/s";
    else                  ss << static_cast<int64_t>(hps) << " H/s";
    return ss.str();
}

double Benchmark::runCPU(const std::string& name, bool useSIMD, uint64_t count) {
    auto engine = HashEngineFactory::create(name, useSIMD);
    if (!engine) return 0.0;

    std::vector<std::string> candidates(count, "benchmark_test");
    std::vector<std::vector<uint8_t>> outputs;

    utils::Timer timer;
    engine->hashBatch(candidates, outputs);
    double elapsed = timer.elapsedSeconds();

    return (elapsed > 0.0) ? (static_cast<double>(count) / elapsed) : 0.0;
}

double Benchmark::runGPU(const std::string& name, uint64_t count, int deviceIndex) {
#ifdef FENRIR_HAS_GPU
    auto engine = HashEngineFactory::create(name, false);
    if (!engine) return 0.0;

    std::string kernelPath = engine->kernelSourcePath();
    std::string kernelFunc = engine->kernelFunctionName();
    if (kernelPath.empty() || kernelFunc.empty()) return 0.0; // No GPU kernel

    auto ctx = std::make_shared<gpu::OpenCLContext>(deviceIndex);
    if (!ctx->isAvailable()) return 0.0;

    gpu::GpuHasher hasher(ctx);
    if (!hasher.initialize(name, kernelPath, kernelFunc)) return 0.0;

    std::vector<std::string> candidates(count, "benchmark_test");
    std::vector<uint32_t> targets(1, 0xFFFFFFFF); // Dummy target
    std::vector<size_t> found;

    utils::Timer timer;
    hasher.hashBatch(candidates, targets, found);
    double elapsed = timer.elapsedSeconds();

    return (elapsed > 0.0) ? (static_cast<double>(count) / elapsed) : 0.0;
#else
    return 0.0;
#endif
}

void Benchmark::printTable(const std::vector<Result>& results) {
    utils::Logger::info("");
    utils::Logger::info("==========================================================");
    utils::Logger::info("  FENRIR BENCHMARK RESULTS");
    utils::Logger::info("==========================================================");

    // Header
    std::ostringstream header;
    header << std::left << std::setw(22) << "Algorithm"
           << std::right << std::setw(20) << "CPU"
           << std::setw(22) << "GPU";
    utils::Logger::info(header.str());
    utils::Logger::info("----------------------------------------------------------");

    for (const auto& r : results) {
        std::ostringstream row;
        row << std::left << std::setw(22) << r.algorithm
            << std::right << std::setw(20) << formatHps(r.cpuHps)
            << std::setw(22) << (r.gpuHps > 0 ? formatHps(r.gpuHps) : "N/A");
        utils::Logger::info(row.str());
    }
    utils::Logger::info("==========================================================");
    utils::Logger::info("");
}

void Benchmark::runAll(const Config& config) {
    utils::Logger::info("Running benchmark (" + std::to_string(config.benchmarkCount) + " candidates per algorithm)...");

    // All registered algorithm names (must match HashEngineFactory::typeFromName)
    std::vector<std::string> algorithms = {
        "md5", "sha1", "sha256", "sha384", "sha512",
        "ntlm", "bcrypt", "scrypt", "sha3", "pbkdf2", "argon2",
        "blake2b", "hmac-md5", "hmac-sha256"
    };

    std::vector<Result> results;

    for (const auto& name : algorithms) {
        Result r;
        r.algorithm = name;

        try {
            r.cpuHps = runCPU(name, config.enableSIMD, config.benchmarkCount);
        } catch (...) {
            r.cpuHps = 0.0;
        }

        try {
            if (!config.cpuOnly) {
                r.gpuHps = runGPU(name, config.benchmarkCount, config.gpuDevice);
            } else {
                r.gpuHps = 0.0;
            }
        } catch (...) {
            r.gpuHps = 0.0;
        }

        results.push_back(r);
        utils::Logger::debug("  " + name + " benchmark complete");
    }

    printTable(results);
}

}
}
