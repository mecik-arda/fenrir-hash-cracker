#pragma once
#include "IHashEngine.hpp"

namespace fenrir { namespace core {

class NTMLEngine : public IHashEngine {
public:
    HashType type() const override { return HashType::NTLM; }
    std::string name() const override { return "NTLM"; }
    std::size_t hashSize() const override { return 16; }
    bool supportsSalt() const override { return false; }
    bool isSlowHash() const override { return false; }
    std::vector<uint8_t> hash(const std::string& input, const std::vector<uint8_t>& salt = {}) const override;
    void hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt = {}) const override;
    std::string kernelSourcePath() const override { return "kernels/ntlm_optimized.cl"; }
    std::string kernelFunctionName() const override { return "ntlm_optimized_crack"; }
    double estimatedHashPerSecond(int gpuComputeUnits) const override { return gpuComputeUnits * 55'000'000.0; }
};

} }
