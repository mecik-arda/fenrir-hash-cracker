#pragma once
#include "IHashEngine.hpp"

namespace fenrir { namespace core {

class SHA512Engine : public IHashEngine {
public:
    HashType type() const override { return HashType::SHA512; }
    std::string name() const override { return "SHA512"; }
    std::size_t hashSize() const override { return 64; }
    bool supportsSalt() const override { return true; }
    bool isSlowHash() const override { return false; }
    std::vector<uint8_t> hash(const std::string& input, const std::vector<uint8_t>& salt = {}) const override;
    void hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt = {}) const override;
    std::string kernelSourcePath() const override { return "kernels/sha512.cl"; }
    std::string kernelFunctionName() const override { return "sha512_crack"; }
    double estimatedHashPerSecond(int gpuComputeUnits) const override { return gpuComputeUnits * 15'000'000.0; }
};

} }
