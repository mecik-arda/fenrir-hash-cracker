#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace fenrir {
namespace core {

constexpr const char* VERSION        = "1.1.0";
constexpr const char* VERSION_STRING = "Fenrir Hash Cracker v1.1.0";
constexpr const char* AUTHOR         = "Fenrir Project";
constexpr const char* DESCRIPTION    = "GPU-accelerated password hash auditing tool";

constexpr std::size_t DEFAULT_FAST_HASH_BATCH_SIZE = 1'000'000;
constexpr std::size_t DEFAULT_SLOW_HASH_BATCH_SIZE = 1'024;
constexpr std::size_t DEFAULT_API_BATCH_SIZE       = 100;
constexpr std::size_t MAX_BATCH_SIZE               = 10'000'000;

constexpr std::size_t DEFAULT_LOCAL_WORK_SIZE = 256;
constexpr std::size_t MAX_WORK_GROUP_SIZE     = 1024;

constexpr int MAX_WORD_LENGTH    = 255;
constexpr int MAX_RULE_LENGTH    = 64;
constexpr int MAX_MEMORY_LENGTH  = 255;

constexpr int DEFAULT_CHECKPOINT_INTERVAL_SECONDS = 30;

constexpr int DEFAULT_API_TIMEOUT_SECONDS  = 10;
constexpr int DEFAULT_API_MAX_RETRIES      = 3;
constexpr double DEFAULT_API_RATE_LIMIT_BUF = 1.1;

constexpr const char* DEFAULT_CONFIG_PATH     = "config/default_config.json";
constexpr const char* DEFAULT_OUTPUT_FILE     = "cracked.txt";
constexpr const char* DEFAULT_CHECKPOINT_FILE = "fenrir.checkpoint";
constexpr const char* ENV_API_KEY             = "FENRIR_API_KEY";
constexpr const char* ENV_CONFIG_PATH         = "FENRIR_CONFIG";

constexpr std::size_t FILE_READ_BUFFER_SIZE = 64 * 1024;

constexpr uint32_t CHECKPOINT_MAGIC = 0x46434B50;
constexpr uint16_t CHECKPOINT_VERSION_MAJOR = 1;
constexpr uint16_t CHECKPOINT_VERSION_MINOR = 0;

constexpr std::size_t MAX_HASH_BYTES  = 64;
constexpr std::size_t MAX_SALT_BYTES  = 128;
constexpr std::size_t MAX_HASH_STRING = 256;

}
}
