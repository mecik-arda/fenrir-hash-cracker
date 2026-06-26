#pragma once
#include "IHashEngine.hpp"
#include <cstdint>

namespace fenrir { namespace core {

class Argon2Engine : public IHashEngine {
public:
    Argon2Engine(int memoryKB = 65536, int iterations = 3, int parallelism = 4);
    HashType type() const override { return HashType::ARGON2; }
    std::string name() const override { return m_name; }
    std::size_t hashSize() const override { return m_tagLength; }
    bool supportsSalt() const override { return true; }
    bool isSlowHash() const override { return true; }
    std::vector<uint8_t> hash(const std::string& input, const std::vector<uint8_t>& salt = {}) const override;
    void hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt = {}) const override;
    std::string kernelSourcePath() const override { return "kernels/argon2.cl"; }
    std::string kernelFunctionName() const override { return "argon2_crack"; }
    double estimatedHashPerSecond(int gpuComputeUnits) const override;

private:
    int m_memoryKB;
    int m_iterations;
    int m_parallelism;
    int m_tagLength = 32;
    std::string m_name;
};

} }
