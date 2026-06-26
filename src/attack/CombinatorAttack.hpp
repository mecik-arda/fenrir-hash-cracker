#pragma once
#include "IAttackMode.hpp"
#include "../utils/FileReader.hpp"
#include <memory>
#include <vector>
#include <string>

namespace fenrir { namespace attack {

class CombinatorAttack : public IAttackMode {
public:
    std::string name() const override { return "Combinator"; }
    void initialize(const core::Config& cfg) override;
    bool nextBatch(std::vector<std::string>& candidates, size_t batchSize) override;
    std::vector<uint8_t> serializeState() const override;
    void deserializeState(const std::vector<uint8_t>& data) override;
    uint64_t totalCandidateEstimate() const override;
    uint64_t candidatesGenerated() const override;
    bool isExhausted() const override;

private:
    std::unique_ptr<utils::FileReader> m_reader1;
    std::unique_ptr<utils::FileReader> m_reader2;
    std::string m_wordlist1Path;
    std::string m_wordlist2Path;
    std::string m_currentWord1;
    uint64_t m_generated = 0;
    uint64_t m_totalEstimate = 0;
    bool m_exhausted = false;

    std::vector<std::string> m_word2Cache;
    size_t m_word2Pos = 0;
};

} }
