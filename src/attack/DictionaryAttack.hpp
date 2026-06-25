#pragma once
#include "IAttackMode.hpp"
#include "../utils/FileReader.hpp"
#include <memory>

namespace fenrir { namespace attack {

class DictionaryAttack : public IAttackMode {
public:
    std::string name() const override { return "Dictionary"; }
    void initialize(const core::Config& cfg) override;
    bool nextBatch(std::vector<std::string>& candidates, size_t batchSize) override;
    std::vector<uint8_t> serializeState() const override;
    void deserializeState(const std::vector<uint8_t>& data) override;
    uint64_t totalCandidateEstimate() const override;
    uint64_t candidatesGenerated() const override;
    bool isExhausted() const override;

private:
    std::unique_ptr<utils::FileReader> m_reader;
    std::string m_wordlistPath;
    uint64_t m_generated = 0;
    uint64_t m_fileSize = 0;
    bool m_exhausted = false;
};

} }
