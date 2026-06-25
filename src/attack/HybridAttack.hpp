#pragma once
#include "IAttackMode.hpp"
#include "../utils/FileReader.hpp"
#include <memory>

namespace fenrir { namespace attack {

class HybridAttack : public IAttackMode {
public:
    std::string name() const override { return "Hybrid"; }
    void initialize(const core::Config& cfg) override;
    bool nextBatch(std::vector<std::string>& candidates, size_t batchSize) override;
    std::vector<uint8_t> serializeState() const override;
    void deserializeState(const std::vector<uint8_t>& data) override;
    uint64_t totalCandidateEstimate() const override;
    uint64_t candidatesGenerated() const override;
    bool isExhausted() const override;

private:
    std::unique_ptr<utils::FileReader> m_reader;
    std::string m_charset;
    int m_minSuffix = 1, m_maxSuffix = 3;
    std::string m_currentWord;
    uint64_t m_generated = 0;
    uint64_t m_totalEstimate = 0;
    bool m_exhausted = false;


    std::vector<size_t> m_suffixIndices;
    int m_currentSuffixLen = 1;
    bool m_suffixExhausted = false;

    void resetSuffixGenerator(int len);
    bool nextSuffix(std::string& suffix);
};

} }
