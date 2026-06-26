#include "HybridAttack.hpp"

namespace fenrir { namespace attack {

void HybridAttack::initialize(const core::Config& cfg) {
    m_reader = std::make_unique<utils::FileReader>(cfg.wordlist);
    m_charset = cfg.charset.empty() ? "0123456789" : cfg.charset;
    m_minSuffix = cfg.minLength;
    m_maxSuffix = cfg.maxLength;

    uint64_t words = m_reader->fileSize() / 8;
    uint64_t combos = 0;
    for (int l=m_minSuffix; l<=m_maxSuffix; l++) {
        uint64_t p=1;
        for (int i=0;i<l;i++) p*=m_charset.size();
        combos += p;
    }
    m_totalEstimate = words * combos;
    m_generated = 0;
    m_exhausted = false;
    m_currentWord.clear();
    m_currentSuffixLen = m_minSuffix;
    m_suffixExhausted = true;
    resetSuffixGenerator(m_minSuffix);
}

void HybridAttack::resetSuffixGenerator(int len) {
    m_suffixIndices.assign(static_cast<size_t>(len), 0);
    m_suffixExhausted = (len == 0);
}

bool HybridAttack::nextSuffix(std::string& suffix) {
    if (m_suffixExhausted) return false;
    suffix.assign(m_suffixIndices.size(), ' ');
    for (size_t i=0;i<m_suffixIndices.size();i++)
        suffix[i] = m_charset[m_suffixIndices[i]];

    for (int i=static_cast<int>(m_suffixIndices.size())-1;i>=0;i--) {
        m_suffixIndices[i]++;
        if (m_suffixIndices[i] < m_charset.size()) return true;
        m_suffixIndices[i] = 0;
    }
    m_suffixExhausted = true;
    return true;
}

bool HybridAttack::nextBatch(std::vector<std::string>& candidates, size_t batchSize) {
    candidates.clear();
    if (m_exhausted) return false;

    while (candidates.size() < batchSize) {
        if (m_currentWord.empty()) {
            auto line = m_reader->nextLine();
            if (!line) { m_exhausted = true; break; }
            m_currentWord = std::move(*line);
            m_currentSuffixLen = m_minSuffix;
            resetSuffixGenerator(m_currentSuffixLen);
        }

        if (m_suffixExhausted) {
            m_currentSuffixLen++;
            if (m_currentSuffixLen > m_maxSuffix) {
                m_currentWord.clear();
                continue;
            }
            resetSuffixGenerator(m_currentSuffixLen);
        }

        std::string suffix;
        nextSuffix(suffix);
        candidates.push_back(m_currentWord + suffix);
    }
    m_generated += candidates.size();
    return !candidates.empty();
}

std::vector<uint8_t> HybridAttack::serializeState() const {
    size_t sz = 16 + 4 + 4 + m_suffixIndices.size() * 8;
    std::vector<uint8_t> d(sz, 0);
    uint64_t offset = m_reader ? m_reader->currentOffset() : 0;
    std::memcpy(d.data(), &offset, 8);
    std::memcpy(d.data()+8, &m_generated, 8);
    int suffixLen = m_currentSuffixLen;
    std::memcpy(d.data()+16, &suffixLen, 4);
    int indicesCount = static_cast<int>(m_suffixIndices.size());
    std::memcpy(d.data()+20, &indicesCount, 4);
    for (size_t i=0; i<m_suffixIndices.size(); i++) {
        uint64_t val = m_suffixIndices[i];
        std::memcpy(d.data()+24+i*8, &val, 8);
    }
    return d;
}

void HybridAttack::deserializeState(const std::vector<uint8_t>& d) {
    if (d.size()<24 || !m_reader) return;
    uint64_t off, gen;
    std::memcpy(&off,d.data(),8);
    std::memcpy(&gen,d.data()+8,8);
    int suffixLen, indicesCount;
    std::memcpy(&suffixLen,d.data()+16,4);
    std::memcpy(&indicesCount,d.data()+20,4);
    if (indicesCount < 0 || indicesCount > 256) indicesCount = 0; // sanity check
    m_reader->seekTo(off);
    m_generated=gen;
    m_exhausted=false;
    m_currentWord.clear();
    m_currentSuffixLen = suffixLen;
    m_suffixIndices.resize(indicesCount);
    if (d.size() >= 24 + static_cast<size_t>(indicesCount)*8) {
        for (int i=0; i<indicesCount; i++) {
            uint64_t val;
            std::memcpy(&val, d.data()+24+i*8, 8);
            m_suffixIndices[i] = static_cast<size_t>(val);
        }
    }
    m_suffixExhausted = false;
}

uint64_t HybridAttack::totalCandidateEstimate() const { return m_totalEstimate; }
uint64_t HybridAttack::candidatesGenerated() const { return m_generated; }
bool HybridAttack::isExhausted() const { return m_exhausted; }

} }
