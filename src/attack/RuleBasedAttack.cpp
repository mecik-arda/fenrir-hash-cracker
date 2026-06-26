#include "RuleBasedAttack.hpp"

namespace fenrir { namespace attack {

void RuleBasedAttack::initialize(const core::Config& cfg) {
    m_wordlistPath = cfg.wordlist;
    m_reader = std::make_unique<utils::FileReader>(m_wordlistPath);
    m_rules = rules::RuleParser::parseFile(cfg.ruleFile);
    m_totalEstimate = (m_reader->fileSize() / 8) * m_rules.size();
    m_generated = 0;
    m_ruleIndex = 0;
    m_exhausted = false;
    m_currentWord.clear();
}

bool RuleBasedAttack::loadNextWord() {
    auto line = m_reader->nextLine();
    if (!line) return false;
    m_currentWord = std::move(*line);
    m_ruleIndex = 0;
    return true;
}

bool RuleBasedAttack::nextBatch(std::vector<std::string>& candidates, size_t batchSize) {
    candidates.clear();
    if (m_exhausted) return false;

    while (candidates.size() < batchSize) {

        if (m_currentWord.empty() || m_ruleIndex >= m_rules.size()) {
            if (!loadNextWord()) {
                m_exhausted = true;
                break;
            }
        }


        size_t rulesRemaining = m_rules.size() - m_ruleIndex;
        size_t needed = batchSize - candidates.size();
        size_t toApply = std::min(rulesRemaining, needed);

        for (size_t i = 0; i < toApply; i++) {
            candidates.emplace_back();
            if (!m_engine.apply(m_rules[m_ruleIndex], m_currentWord, candidates.back())) {
                candidates.pop_back();
            }
            m_ruleIndex++;
        }
    }

    m_generated += candidates.size();
    return !candidates.empty();
}

std::vector<uint8_t> RuleBasedAttack::serializeState() const {
    std::vector<uint8_t> d(24,0);
    uint64_t offset = m_reader ? m_reader->currentOffset() : 0;
    auto w = [&](size_t o, uint64_t v){std::memcpy(d.data()+o,&v,8);};
    w(0, offset);
    w(8, m_ruleIndex);
    w(16, m_generated);
    return d;
}

void RuleBasedAttack::deserializeState(const std::vector<uint8_t>& d) {
    if (d.size()<24 || !m_reader) return;
    uint64_t off, ri, gen;
    std::memcpy(&off,d.data(),8);
    std::memcpy(&ri,d.data()+8,8);
    std::memcpy(&gen,d.data()+16,8);
    m_reader->seekTo(off);
    m_ruleIndex = static_cast<size_t>(ri);
    m_generated = gen;
    m_exhausted = false;
    m_currentWord.clear();
}

uint64_t RuleBasedAttack::totalCandidateEstimate() const { return m_totalEstimate; }
uint64_t RuleBasedAttack::candidatesGenerated() const { return m_generated; }
bool RuleBasedAttack::isExhausted() const { return m_exhausted; }

} }
