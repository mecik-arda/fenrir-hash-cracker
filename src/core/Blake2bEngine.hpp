#pragma once
#include "IHashEngine.hpp"

namespace fenrir { namespace core {

class Blake2bEngine : public IHashEngine {
public:
    explicit Blake2bEngine(int digestBytes = 64);

    HashType type() const override { return HashType::BLAKE2B; }
    std::string name() const override { return m_name; }
    std::size_t hashSize() const override { return static_cast<size_t>(m_digestSize); }
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
        { return gpuComputeUnits * 20'000'000.0; }

private:
    int m_digestSize;
    std::string m_name;
};

} }
