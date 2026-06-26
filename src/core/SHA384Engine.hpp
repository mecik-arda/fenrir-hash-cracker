#pragma once
#include "IHashEngine.hpp"

namespace fenrir { namespace core {

class SHA384Engine : public IHashEngine {
public:
    HashType type() const override { return HashType::SHA384; }
    std::string name() const override { return "SHA384"; }
    std::size_t hashSize() const override { return 48; }
    bool supportsSalt() const override { return true; }
    bool isSlowHash() const override { return false; }
    std::vector<uint8_t> hash(const std::string& input,
        const std::vector<uint8_t>& salt = {}) const override;
    void hashBatch(const std::vector<std::string>& inputs,
        std::vector<std::vector<uint8_t>>& outputs,
        const std::vector<uint8_t>& salt = {}) const override;
    std::string kernelSourcePath() const override { return "kernels/sha384.cl"; }
    std::string kernelFunctionName() const override { return "sha384_crack"; }
    double estimatedHashPerSecond(int gpuComputeUnits) const override
        { return gpuComputeUnits * 15'000'000.0; }
};

} }
