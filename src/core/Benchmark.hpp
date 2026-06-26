#pragma once

#include "Config.hpp"
#include <vector>
#include <string>

namespace fenrir {
namespace core {

class Benchmark {
public:
    static void runAll(const Config& config);

private:
    struct Result {
        std::string algorithm;
        double cpuHps;
        double gpuHps;
    };

    static double runCPU(const std::string& name, bool useSIMD, uint64_t count);
    static double runGPU(const std::string& name, uint64_t count, int deviceIndex);
    static void printTable(const std::vector<Result>& results);
};

}
}
