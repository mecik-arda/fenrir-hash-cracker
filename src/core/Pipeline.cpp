#include "Pipeline.hpp"
#include "HashEngineFactory.hpp"
#include "ResultWriter.hpp"
#include "../attack/AttackRegistry.hpp"
#include "../utils/Logger.hpp"
#include "../utils/HashParser.hpp"
#include "../utils/ShadowParser.hpp"
#include "../utils/PlistParser.hpp"
#include "../utils/Checkpoint.hpp"
#include "../utils/SignalHandler.hpp"
#include "../utils/Timer.hpp"
#ifdef FENRIR_HAS_GPU
#include "../gpu/GpuHasher.hpp"
#include "../gpu/GpuStats.hpp"
#include "../gpu/OpenCLContext.hpp"
#include "../gpu/AsyncGpuHasher.hpp"
#else
namespace fenrir { namespace gpu {
    struct GpuStats {
        void addAttempts(uint64_t){}
        void updateTimer(double){}
        std::string hashRateFormatted() const { return ""; }
        std::string etaFormatted(uint64_t) const { return ""; }
    };
    struct OpenCLContext {
        OpenCLContext(int) {}
        bool isAvailable() const { return false; }
    };
    struct GpuHasher {
        GpuHasher(std::shared_ptr<OpenCLContext>) {}
        bool isReady() const { return false; }
        void hashBatch(const std::vector<std::string>&, const std::vector<uint32_t>&, std::vector<size_t>&) {}
        bool initialize(const std::string&, const std::string&, const std::string&) { return false; }
    };
}}
#endif

#ifdef FENRIR_HAS_API
#include "../api/ApiManager.hpp"
#else
namespace fenrir { namespace api {
    struct ApiQueryResult { bool found = false; std::string plaintext = ""; };
    struct ApiManager {
        std::vector<ApiQueryResult> queryWithFallback(const std::vector<std::string>&, const std::string&, const std::string&, int=3, int=5) { return {}; }
    };
}}
#endif
#include "../cpu_simd/SIMDDetector.hpp"
#include "../ui/ProgressDisplay.hpp"

#include <sstream>
#include <iomanip>
#include <thread>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <fstream>
#include <map>

namespace fenrir {
namespace core {

static std::string toHex(const std::vector<uint8_t>& bytes) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto b : bytes) ss << std::setw(2) << static_cast<int>(b);
    return ss.str();
}

static bool hashMatch(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    return std::memcmp(a.data(), b.data(), a.size()) == 0;
}

Pipeline::Pipeline(const Config& config)
    : m_config(config)
{
    m_engine = HashEngineFactory::create(config.hashMode, config.enableSIMD);
    if (!m_engine) {
        throw std::runtime_error("Unknown hash mode: " + config.hashMode);
    }
    if (config.enableSIMD) {
        utils::Logger::info("SIMD: " + fenrir::simd::SIMDDetector::levelName());
    }
}

int Pipeline::run() {
    utils::Logger::info("Initializing pipeline...");


    m_targets.clear();
    auto algo = m_engine->type();
    for (const auto& file : m_config.hashFiles) {
        // Try shadow format first, fall back to standard hash file
        auto shadowEntries = utils::ShadowParser::parseFile(file);
        if (!shadowEntries.empty()) {
            for (const auto& se : shadowEntries) {
                std::string detectedMode = utils::ShadowParser::detectHashMode(se.hashString);
                if (!detectedMode.empty() && m_config.hashMode.empty()) {
                    // Auto-detect: override engine to match shadow hash type
                    utils::Logger::info("Shadow detected: " + se.username +
                                       " -> " + detectedMode + " hash");
                }
                // Use the hash string directly — HashParser handles $id$ prefixed hashes
                auto target = utils::HashParser::parse(se.hashString, algo);
                target.username = se.username;
                m_targets.push_back(target);
            }
        } else {
            // Try plist format
            auto plistEntries = utils::PlistParser::parseFile(file);
            if (!plistEntries.empty()) {
                for (const auto& pe : plistEntries) {
                    auto target = utils::HashParser::parse(pe.hashString, algo);
                    target.username = pe.username;
                    m_targets.push_back(target);
                }
            } else {
                auto parsed = utils::HashParser::parseFile(file, algo);
                m_targets.insert(m_targets.end(), parsed.begin(), parsed.end());
            }
        }
    }
    for (const auto& h : m_config.inlineHashes) {
        auto t = utils::HashParser::parse(h, algo);
        // Check if inline hash looks like a shadow entry
        auto se = utils::ShadowParser::parseLine(h);
        if (se) { t.username = se->username; }
        m_targets.push_back(t);
    }

    if (m_targets.empty()) {
        utils::Logger::error("No target hashes loaded");
        return 0;
    }
    utils::Logger::info("Loaded " + std::to_string(m_targets.size()) + " target hash(es)");

    // --- Potfile: pre-load already-cracked hashes ---
    {
        std::ifstream pf(m_config.potfile);
        if (pf.is_open()) {
            std::string line;
            int preCracked = 0;
            while (std::getline(pf, line)) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string h = line.substr(0, colon);
                    if (!h.empty() && h.back() == '\r') h.pop_back();
                    for (auto& t : m_targets) {
                        if (!t.cracked && t.hex() == h) {
                            t.cracked = true;
                            preCracked++;
                            break;
                        }
                    }
                }
            }
            if (preCracked > 0) {
                utils::Logger::info("Pre-cracked " + std::to_string(preCracked) +
                                   " hash(es) from potfile: " + m_config.potfile);
            }
        }
    }

    auto attack = attack::AttackRegistry::create(m_config.attack);
    if (!attack) {
        utils::Logger::error("Unknown attack mode: " + m_config.attack);
        return 0;
    }
    attack->initialize(m_config);


    if (m_config.resume && utils::Checkpoint::exists(m_config.checkpointFile)) {
        auto ck = utils::Checkpoint::load(m_config.checkpointFile);
        if (ck) {
            attack->deserializeState(ck->attackState);
            m_attempts = ck->candidatesGenerated;
            utils::Logger::info("Resumed from checkpoint: " +
                               std::to_string(m_attempts) + " candidates already tested");
        }
    }


    std::shared_ptr<gpu::GpuHasher> gpuHasher;
    std::shared_ptr<gpu::OpenCLContext> gpuContext;
    bool useGpu = !m_config.cpuOnly;

    if (useGpu) {
        gpuContext = std::make_shared<gpu::OpenCLContext>(m_config.gpuDevice);
        if (gpuContext->isAvailable()) {
            gpuHasher = std::make_shared<gpu::GpuHasher>(gpuContext);
            bool ok = gpuHasher->initialize(
                m_engine->name(),
                m_engine->kernelSourcePath(),
                m_engine->kernelFunctionName());
            if (!ok) {
                utils::Logger::warn("GPU init failed — falling back to CPU");
                gpuHasher.reset();
            }
        } else {
            utils::Logger::info("No GPU available — using CPU");
        }
    }

    if (!gpuHasher) {
        utils::Logger::info("Running in CPU mode");
    }


    ResultWriter writer(m_config.outputFile, m_config.outputAppend);
    std::ofstream potStream(m_config.potfile, std::ios::app);

    // TUI progress display
    std::unique_ptr<ui::ProgressDisplay> tui;
    if (!m_config.noTui) {
        std::string engineInfo = m_config.cpuOnly ? "CPU" : "GPU";
        if (m_config.enableSIMD) engineInfo += "+SIMD";
        tui = std::make_unique<ui::ProgressDisplay>(
            m_engine->name(), attack->name(), engineInfo, attack->totalCandidateEstimate());
    }

    gpu::GpuStats stats;
    utils::Timer timer;
    auto lastCheckpoint = timer.elapsed();
    bool interrupted = false;
    auto lastTuiUpdate = timer.elapsed();


    utils::SignalHandler::install([&]() {
        interrupted = true;
    });


    if (m_config.attack == "api") {
        return runApiLookup();
    }


    utils::Logger::info("Starting attack...");
    utils::Logger::info("");

    size_t batchSize = m_engine->isSlowHash()
        ? m_config.slowHashBatchSize
        : m_config.fastHashBatchSize;

    uint64_t totalEstimate = attack->totalCandidateEstimate();
    if (totalEstimate > 0 && m_config.noTui) {
        utils::Logger::info("Estimated keyspace: " + std::to_string(totalEstimate));
    }

    std::map<std::vector<uint8_t>, std::map<std::vector<uint8_t>, TargetHash*>> saltHashGroups;
    for (auto& t : m_targets) {
        if (!t.cracked) saltHashGroups[t.salt()][t.hash()] = &t;
    }

    while (!interrupted && !attack->isExhausted()) {

        size_t remaining = 0;
        for (const auto& t : m_targets) {
            if (!t.cracked) remaining++;
        }
        if (remaining == 0) {
            utils::Logger::info("All targets cracked!");
            break;
        }


        std::vector<std::string> candidates;
        if (!attack->nextBatch(candidates, batchSize)) {
            break;
        }


        if (gpuHasher && gpuHasher->isReady()) {

            std::vector<uint32_t> prefixes;
            for (const auto& t : m_targets) {
                if (!t.cracked && !t.hash().empty()) {
                    uint32_t prefix = 0;
                    std::memcpy(&prefix, t.hash().data(), std::min(sizeof(uint32_t), t.hash().size()));
                    prefixes.push_back(prefix);
                } else {
                    prefixes.push_back(0xFFFFFFFF);
                }
            }

            std::vector<size_t> found;
            gpuHasher->hashBatch(candidates, prefixes, found);


            for (size_t idx : found) {
                if (idx >= candidates.size()) continue;
                const auto& candidate = candidates[idx];

                for (auto& kv : saltHashGroups) {
                    const auto& salt = kv.first;
                    auto& hashMap = kv.second;
                    if (hashMap.empty()) continue;

                    auto computedHash = m_engine->hash(candidate, salt);
                    auto it = hashMap.find(computedHash);
                    if (it != hashMap.end()) {
                        auto t = it->second;
                        if (!t->cracked) {
                            t->cracked = true;
                            writer.write(t->hex(), candidate);
                            std::string display = t->username.empty() ? t->hex() : t->username;
                            utils::Logger::info("CRACKED: " + display + " -> " + candidate);
                            if (tui) tui->onCrack(display, candidate);
                            if (potStream.is_open()) { potStream << t->hex() << ":" << candidate << "\n"; potStream.flush(); }
                            m_cracked++;
                            hashMap.erase(it);
                        }
                    }
                }
            }
        } else {

            for (const auto& candidate : candidates) {
                for (auto& kv : saltHashGroups) {
                    const auto& salt = kv.first;
                    auto& hashMap = kv.second;
                    if (hashMap.empty()) continue;

                    auto computedHash = m_engine->hash(candidate, salt);
                    auto it = hashMap.find(computedHash);
                    if (it != hashMap.end()) {
                        auto t = it->second;
                        if (!t->cracked) {
                            t->cracked = true;
                            writer.write(t->hex(), candidate);
                            std::string display = t->username.empty() ? t->hex() : t->username;
                            utils::Logger::info("CRACKED: " + display + " -> " + candidate);
                            if (tui) tui->onCrack(display, candidate);
                            if (potStream.is_open()) { potStream << t->hex() << ":" << candidate << "\n"; potStream.flush(); }
                            m_cracked++;
                            hashMap.erase(it);
                        }
                    }
                }
            }
        }

        m_attempts += candidates.size();
        stats.addAttempts(candidates.size());
        stats.updateTimer(timer.elapsedSeconds());


        auto now = timer.elapsed();
        if (std::chrono::duration<double>(now - lastCheckpoint).count() >= 5.0) {
            lastCheckpoint = now;

            size_t rem = 0;
            for (const auto& t : m_targets) if (!t.cracked) rem++;

            uint64_t remainingEstimate = totalEstimate > 0
                ? totalEstimate - m_attempts
                : rem * (m_attempts / std::max(uint64_t(1), m_cracked + 1));

            if (tui) {
                // Live TUI update
                uint64_t remainingEstimate = totalEstimate > 0
                    ? totalEstimate - m_attempts
                    : rem * (m_attempts / std::max(uint64_t(1), m_cracked + 1));
                tui->update(m_attempts, m_cracked, rem,
                           stats.hashRateFormatted(),
                           stats.etaFormatted(remainingEstimate),
                           timer.elapsedFormatted());
            } else {
                std::ostringstream progress;
                progress << "\r[" << timer.elapsedFormatted() << "] "
                         << stats.hashRateFormatted() << " | "
                         << m_attempts << " tested | "
                         << m_cracked << " cracked | "
                         << rem << " remaining | ETA: "
                         << stats.etaFormatted(remainingEstimate);
                utils::Logger::info(progress.str());
            }
        }


        auto checkpointElapsed = std::chrono::duration<double>(timer.elapsed() - lastCheckpoint).count();
        if (checkpointElapsed >= static_cast<double>(m_config.checkpointIntervalSeconds)) {
            utils::Checkpoint::Data ck;
            ck.algorithm = static_cast<uint32_t>(m_engine->type());
            ck.attackState = attack->serializeState();
            ck.candidatesGenerated = m_attempts;
            utils::Checkpoint::save(m_config.checkpointFile, ck);
        }

        // Poll signal handler at safe points
        utils::SignalHandler::checkAndInvoke();
        if (interrupted) {
            utils::Logger::info("Interrupted! Saving checkpoint...");
            utils::Checkpoint::Data ck;
            ck.algorithm = static_cast<uint32_t>(m_engine->type());
            ck.attackState = attack->serializeState();
            ck.candidatesGenerated = m_attempts;
            utils::Checkpoint::save(m_config.checkpointFile, ck);
            utils::Logger::info("Checkpoint saved. Resume with --resume");
            break;
        }
    }


    auto elapsed = timer.elapsedSeconds();
    std::string speedStr;
    if (elapsed > 0) {
        double hps = static_cast<double>(m_attempts) / elapsed;
        std::ostringstream ss;
        if (hps >= 1e9) ss << std::fixed << std::setprecision(2) << (hps/1e9) << " GH/s";
        else if (hps >= 1e6) ss << std::fixed << std::setprecision(2) << (hps/1e6) << " MH/s";
        else if (hps >= 1e3) ss << std::fixed << std::setprecision(2) << (hps/1e3) << " kH/s";
        else ss << static_cast<int64_t>(hps) << " H/s";
        speedStr = ss.str();
    }

    if (tui) {
        tui->summary(m_attempts, m_cracked, timer.elapsedFormatted(), speedStr);
    }
    utils::Logger::info("");
    utils::Logger::info("================================================");
    utils::Logger::info("Attack complete!");
    utils::Logger::info("  Time:      " + timer.elapsedFormatted());
    utils::Logger::info("  Tested:    " + std::to_string(m_attempts));
    utils::Logger::info("  Cracked:   " + std::to_string(m_cracked));
    if (!speedStr.empty()) {
        utils::Logger::info("  Speed:     " + speedStr);
    }


    if (!interrupted && !m_config.resume) {
        utils::Checkpoint::clear(m_config.checkpointFile);
    }

    return static_cast<int>(m_cracked);
}

int Pipeline::runApiLookup() {
    utils::Logger::info("Running API lookup mode...");
    fenrir::api::ApiManager apiMgr;
    utils::Timer timer;
    ResultWriter writer(m_config.outputFile, m_config.outputAppend);

    for (auto& t : m_targets) {
        if (t.cracked) continue;
        m_attempts++;

        auto results = apiMgr.queryWithFallback(
            {t.hex()}, m_config.apiProvider, m_config.apiKey,
            m_config.apiMaxRetries, m_config.apiTimeoutSeconds);

        for (const auto& r : results) {
            if (r.found && !r.plaintext.empty()) {
                t.cracked = true;
                m_cracked++;
                writer.write(t.hex(), r.plaintext);
                utils::Logger::info("API CRACKED: " + t.hex() + " -> " + r.plaintext);
            }
        }

        if (utils::SignalHandler::interrupted()) {
            utils::Checkpoint::save(m_config.checkpointFile, {});
            break;
        }
    }

    auto elapsed = timer.elapsedSeconds();
    utils::Logger::info("");
    utils::Logger::info("================================================");
    utils::Logger::info("API lookup complete!");
    utils::Logger::info("  Queried:   " + std::to_string(m_attempts));
    utils::Logger::info("  Cracked:   " + std::to_string(m_cracked));
    if (elapsed > 0) {
        utils::Logger::info("  Time:      " + timer.elapsedFormatted());
    }
    if (!m_config.resume) utils::Checkpoint::clear(m_config.checkpointFile);
    return static_cast<int>(m_cracked);
}

uint64_t Pipeline::attempts() const { return m_attempts; }
uint64_t Pipeline::cracked() const  { return m_cracked; }
double Pipeline::hashRate() const   { return 0.0; }
double Pipeline::etaSeconds() const { return 0.0; }

}
}
