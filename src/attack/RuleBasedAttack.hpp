#pragma once
#include "IAttackMode.hpp"
#include "../rules/RuleParser.hpp"
#include "../rules/RuleEngine.hpp"
#include "../utils/FileReader.hpp"
#include <memory>
#include <vector>

namespace fenrir { namespace attack {

class RuleBasedAttack : public IAttackMode {
public:
    std::string name() const override { return "Rule-Based"; }
    void initialize(const core::Config& cfg) override;
    bool nextBatch(std::vector<std::string>& candidates, size_t batchSize) override;
    std::vector<uint8_t> serializeState() const override;
    void deserializeState(const std::vector<uint8_t>& data) override;
    uint64_t totalCandidateEstimate() const override;
    uint64_t candidatesGenerated() const override;
    bool isExhausted() const override;

private:
    std::unique_ptr<utils::FileReader> m_reader;
    std::vector<rules::CompiledRule> m_rules;
    rules::RuleEngine m_engine;
    std::string m_wordlistPath;
    std::string m_currentWord;
    size_t m_ruleIndex = 0;
    uint64_t m_generated = 0;
    uint64_t m_totalEstimate = 0;
    bool m_exhausted = false;

    bool loadNextWord();
};

} }
