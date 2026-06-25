#include "Checkpoint.hpp"
#include "../core/Constants.hpp"

#include <fstream>
#include <stdexcept>

namespace fenrir {
namespace utils {

void Checkpoint::save(const std::string& path, const Data& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot write checkpoint: " + path);
    }


    file.write(reinterpret_cast<const char*>(&core::CHECKPOINT_MAGIC), 4);
    file.write(reinterpret_cast<const char*>(&core::CHECKPOINT_VERSION_MAJOR), 2);
    file.write(reinterpret_cast<const char*>(&core::CHECKPOINT_VERSION_MINOR), 2);


    file.write(reinterpret_cast<const char*>(&data.algorithm), 4);


    uint32_t targetSize = static_cast<uint32_t>(data.targets.size());
    file.write(reinterpret_cast<const char*>(&targetSize), 4);
    if (!data.targets.empty()) {
        file.write(reinterpret_cast<const char*>(data.targets.data()), targetSize);
    }


    uint32_t stateSize = static_cast<uint32_t>(data.attackState.size());
    file.write(reinterpret_cast<const char*>(&stateSize), 4);
    if (!data.attackState.empty()) {
        file.write(reinterpret_cast<const char*>(data.attackState.data()), stateSize);
    }


    file.write(reinterpret_cast<const char*>(&data.candidatesGenerated), 8);
    file.write(reinterpret_cast<const char*>(&data.startedAt), 8);


    uint32_t crc = 0;
    file.write(reinterpret_cast<const char*>(&crc), 4);
}

std::optional<Checkpoint::Data> Checkpoint::load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }


    uint32_t magic = 0;
    file.read(reinterpret_cast<char*>(&magic), 4);
    if (magic != core::CHECKPOINT_MAGIC) {
        return std::nullopt;
    }


    uint16_t verMajor = 0, verMinor = 0;
    file.read(reinterpret_cast<char*>(&verMajor), 2);
    file.read(reinterpret_cast<char*>(&verMinor), 2);

    if (verMajor != core::CHECKPOINT_VERSION_MAJOR) {
        return std::nullopt;
    }

    Data data;


    file.read(reinterpret_cast<char*>(&data.algorithm), 4);


    uint32_t targetSize = 0;
    file.read(reinterpret_cast<char*>(&targetSize), 4);
    data.targets.resize(targetSize);
    if (targetSize > 0) {
        file.read(reinterpret_cast<char*>(data.targets.data()), targetSize);
    }


    uint32_t stateSize = 0;
    file.read(reinterpret_cast<char*>(&stateSize), 4);
    data.attackState.resize(stateSize);
    if (stateSize > 0) {
        file.read(reinterpret_cast<char*>(data.attackState.data()), stateSize);
    }


    file.read(reinterpret_cast<char*>(&data.candidatesGenerated), 8);
    file.read(reinterpret_cast<char*>(&data.startedAt), 8);


    return data;
}

void Checkpoint::clear(const std::string& path) {
    std::remove(path.c_str());
}

bool Checkpoint::exists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

}
}
