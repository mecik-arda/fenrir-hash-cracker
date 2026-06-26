#include "Checkpoint.hpp"
#include "../core/Constants.hpp"

#include <fstream>
#include <stdexcept>
#include <cstdio>
#include <vector>

namespace {

// Simple CRC32 for checkpoint integrity
uint32_t computeCRC32(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        }
    }
    return crc ^ 0xFFFFFFFF;
}

}

namespace fenrir {
namespace utils {

void Checkpoint::save(const std::string& path, const Data& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot write checkpoint: " + path);
    }

    // Collect all data for CRC computation
    std::vector<uint8_t> crcData;
    auto writeBytes = [&](const void* ptr, size_t size) {
        auto* p = reinterpret_cast<const uint8_t*>(ptr);
        crcData.insert(crcData.end(), p, p + size);
    };

    writeBytes(&core::CHECKPOINT_MAGIC, 4);
    writeBytes(&core::CHECKPOINT_VERSION_MAJOR, 2);
    writeBytes(&core::CHECKPOINT_VERSION_MINOR, 2);
    writeBytes(&data.algorithm, 4);

    uint32_t targetSize = static_cast<uint32_t>(data.targets.size());
    writeBytes(&targetSize, 4);
    if (!data.targets.empty()) {
        writeBytes(data.targets.data(), targetSize);
    }

    uint32_t stateSize = static_cast<uint32_t>(data.attackState.size());
    writeBytes(&stateSize, 4);
    if (!data.attackState.empty()) {
        writeBytes(data.attackState.data(), stateSize);
    }

    writeBytes(&data.candidatesGenerated, 8);
    writeBytes(&data.startedAt, 8);

    // Write data to file
    file.write(reinterpret_cast<const char*>(crcData.data()), crcData.size());

    // Compute and write CRC32
    uint32_t crc = computeCRC32(crcData.data(), crcData.size());
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
