#pragma once
#include "IHashEngine.hpp"

namespace fenrir { namespace core {

class SHA1Engine : public IHashEngine {
public:
    HashType type() const override { return HashType::SHA1; }
    std::string name() const override { return "SHA1"; }
    std::size_t hashSize() const override { return 20; }
    bool supportsSalt() const override { return true; }
    bool isSlowHash() const override { return false; }
    std::vector<uint8_t> hash(const std::string& input, const std::vector<uint8_t>& salt = {}) const override;
    void hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt = {}) const override;
    std::string kernelSourcePath() const override { return "kernels/sha1_optimized.cl"; }
    std::string kernelFunctionName() const override { return "sha1_optimized_crack"; }
    double estimatedHashPerSecond(int gpuComputeUnits) const override { return gpuComputeUnits * 40'000'000.0; }
};

} }
