#pragma once
#include "IHashEngine.hpp"

namespace fenrir { namespace core {

class PBKDF2Engine : public IHashEngine {
public:
    PBKDF2Engine(int iterations = 10000, int keyLength = 32);
    HashType type() const override { return HashType::PBKDF2; }
    std::string name() const override { return m_name; }
    std::size_t hashSize() const override { return (size_t)m_keyLength; }
    bool supportsSalt() const override { return true; }
    bool isSlowHash() const override { return true; }
    std::vector<uint8_t> hash(const std::string& input, const std::vector<uint8_t>& salt = {}) const override;
    void hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt = {}) const override;
    std::string kernelSourcePath() const override { return "kernels/pbkdf2.cl"; }
    std::string kernelFunctionName() const override { return "pbkdf2_crack"; }
    double estimatedHashPerSecond(int gpuComputeUnits) const override;

private:
    int m_iterations;
    int m_keyLength;
    std::string m_name;
};

} }
