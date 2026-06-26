#pragma once
#include "IHashEngine.hpp"

namespace fenrir { namespace core {

class SHA3Engine : public IHashEngine {
public:
    explicit SHA3Engine(int bitStrength = 256);
    HashType type() const override { return HashType::SHA3; }
    std::string name() const override { return m_name; }
    std::size_t hashSize() const override { return m_bitStrength / 8; }
    bool supportsSalt() const override { return true; }
    bool isSlowHash() const override { return false; }
    std::vector<uint8_t> hash(const std::string& input, const std::vector<uint8_t>& salt = {}) const override;
    void hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt = {}) const override;
    std::string kernelSourcePath() const override { return "kernels/sha3.cl"; }
    std::string kernelFunctionName() const override { return "sha3_crack"; }
    double estimatedHashPerSecond(int gpuComputeUnits) const override;

private:
    int m_bitStrength;
    std::string m_name;
};

} }
