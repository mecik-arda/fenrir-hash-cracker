#include "CombinatorAttack.hpp"
#include <cstring>

namespace fenrir { namespace attack {

void CombinatorAttack::initialize(const core::Config& cfg) {
    m_wordlist1Path = cfg.wordlist;
    m_wordlist2Path = cfg.wordlist2;
    m_reader1 = std::make_unique<utils::FileReader>(m_wordlist1Path);
    m_reader2 = std::make_unique<utils::FileReader>(m_wordlist2Path);

    uint64_t est1 = m_reader1->fileSize() / 8;
    uint64_t est2 = m_reader2->fileSize() / 8;
    m_totalEstimate = est1 * est2;
    m_generated = 0;
    m_exhausted = false;
    m_currentWord1.clear();
    m_word2Cache.clear();
    m_word2Pos = 0;
}

bool CombinatorAttack::nextBatch(std::vector<std::string>& candidates, size_t batchSize) {
    candidates.clear();
    if (m_exhausted) return false;

    while (candidates.size() < batchSize && !m_exhausted) {
        if (m_currentWord1.empty()) {
            auto line = m_reader1->nextLine();
            if (!line) { m_exhausted = true; break; }
            m_currentWord1 = std::move(*line);
            m_reader2->seekTo(0);
            m_word2Cache.clear();
            m_word2Pos = 0;
            m_word2Cache = m_reader2->readLines(4096);
        }

        while (m_word2Pos < m_word2Cache.size() && candidates.size() < batchSize) {
            candidates.push_back(m_currentWord1 + m_word2Cache[m_word2Pos++]);
        }

        if (m_word2Pos >= m_word2Cache.size()) {
            m_word2Cache = m_reader2->readLines(4096);
            m_word2Pos = 0;
            if (m_word2Cache.empty()) {
                m_currentWord1.clear(); // Will load next word1 on next iteration
            }
        }
    }

    m_generated += candidates.size();
    return !candidates.empty();
}

std::vector<uint8_t> CombinatorAttack::serializeState() const {
    std::vector<uint8_t> d(32, 0);
    uint64_t off1 = m_reader1 ? m_reader1->currentOffset() : 0;
    uint64_t off2 = m_reader2 ? m_reader2->currentOffset() : 0;
    std::memcpy(d.data(), &off1, 8);
    std::memcpy(d.data() + 8, &off2, 8);
    std::memcpy(d.data() + 16, &m_word2Pos, 8);
    std::memcpy(d.data() + 24, &m_generated, 8);
    return d;
}

void CombinatorAttack::deserializeState(const std::vector<uint8_t>& d) {
    if (d.size() < 32) return;
    uint64_t off1, off2, w2pos, gen;
    std::memcpy(&off1, d.data(), 8);
    std::memcpy(&off2, d.data() + 8, 8);
    std::memcpy(&w2pos, d.data() + 16, 8);
    std::memcpy(&gen, d.data() + 24, 8);
    if (m_reader1) m_reader1->seekTo(off1);
    if (m_reader2) m_reader2->seekTo(off2);
    m_word2Pos = static_cast<size_t>(w2pos);
    m_generated = gen;
    m_exhausted = false;
    m_currentWord1.clear();
    m_word2Cache.clear();
}

uint64_t CombinatorAttack::totalCandidateEstimate() const { return m_totalEstimate; }
uint64_t CombinatorAttack::candidatesGenerated() const { return m_generated; }
bool CombinatorAttack::isExhausted() const { return m_exhausted; }

} }
