#pragma once
#include "IHashEngine.hpp"

namespace fenrir { namespace core {

class HMACSHA256Engine : public IHashEngine {
public:
    HashType type() const override { return HashType::HMAC_SHA256; }
    std::string name() const override { return "HMAC-SHA256"; }
    std::size_t hashSize() const override { return 32; }
    bool supportsSalt() const override { return true; }
    bool isSlowHash() const override { return false; }
    std::vector<uint8_t> hash(const std::string& input,
        const std::vector<uint8_t>& salt = {}) const override;
    void hashBatch(const std::vector<std::string>& inputs,
        std::vector<std::vector<uint8_t>>& outputs,
        const std::vector<uint8_t>& salt = {}) const override;
    std::string kernelSourcePath() const override { return ""; }
    std::string kernelFunctionName() const override { return ""; }
    double estimatedHashPerSecond(int gpuComputeUnits) const override
        { return gpuComputeUnits * 12'000'000.0; }
};

} }
