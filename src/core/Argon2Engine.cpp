#include "Argon2Engine.hpp"
#include <argon2.h>
#include <stdexcept>

namespace fenrir { namespace core {

Argon2Engine::Argon2Engine(int memoryKB, int iterations, int parallelism)
    : m_memoryKB(memoryKB), m_iterations(iterations), m_parallelism(parallelism) {
    m_name = "Argon2id:" + std::to_string(memoryKB) + "K:" + std::to_string(iterations);
}

std::vector<uint8_t> Argon2Engine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    if (salt.empty()) {
        return {};
    }
    
    // The "salt" buffer holds the full target hash string
    std::string encoded(salt.begin(), salt.end());
    
    // argon2id_verify automatically parses the $argon2id$... string and verifies it
    int rc = argon2id_verify(encoded.c_str(), input.data(), input.size());
    
    if (rc == ARGON2_OK) {
        // Return the exact encoded string so the Pipeline finds a match!
        return salt;
    }
    
    return {};
}

void Argon2Engine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (size_t i = 0; i < inputs.size(); i++) outputs[i] = hash(inputs[i], salt);
}

double Argon2Engine::estimatedHashPerSecond(int gpuComputeUnits) const { return gpuComputeUnits * 10.0; }

} }
