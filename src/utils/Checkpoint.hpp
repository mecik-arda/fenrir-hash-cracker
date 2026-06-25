#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace fenrir {
namespace utils {

class Checkpoint {
public:
    struct Data {
        uint32_t algorithm;
        std::vector<uint8_t> targets;
        std::vector<uint8_t> attackState;
        uint64_t candidatesGenerated = 0;
        uint64_t startedAt = 0;
    };


    static void save(const std::string& path, const Data& data);



    static std::optional<Data> load(const std::string& path);


    static void clear(const std::string& path);


    static bool exists(const std::string& path);
};

}
}
