#include "DictionaryAttack.hpp"

namespace fenrir { namespace attack {

void DictionaryAttack::initialize(const core::Config& cfg) {
    m_wordlistPath = cfg.wordlist;
    m_reader = std::make_unique<utils::FileReader>(m_wordlistPath);
    m_fileSize = m_reader->fileSize();
    m_generated = 0;
    m_exhausted = false;
}

bool DictionaryAttack::nextBatch(std::vector<std::string>& candidates, size_t batchSize) {
    candidates.clear();
    if (m_exhausted || !m_reader) return false;
    candidates = m_reader->readLines(batchSize);
    m_generated += candidates.size();
    if (candidates.size() < batchSize) m_exhausted = true;
    return !candidates.empty();
}

std::vector<uint8_t> DictionaryAttack::serializeState() const {
    std::vector<uint8_t> data;
    uint64_t offset = m_reader ? m_reader->currentOffset() : 0;
    auto* p = reinterpret_cast<const uint8_t*>(&offset);
    data.insert(data.end(), p, p + 8);
    p = reinterpret_cast<const uint8_t*>(&m_generated);
    data.insert(data.end(), p, p + 8);
    return data;
}

void DictionaryAttack::deserializeState(const std::vector<uint8_t>& d) {
    if (d.size() < 16 || !m_reader) return;
    uint64_t offset, gen;
    std::memcpy(&offset, d.data(), 8);
    std::memcpy(&gen, d.data() + 8, 8);
    m_reader->seekTo(offset);
    m_generated = gen;
    m_exhausted = false;
}

uint64_t DictionaryAttack::totalCandidateEstimate() const { return m_fileSize / 8; }
uint64_t DictionaryAttack::candidatesGenerated() const { return m_generated; }
bool DictionaryAttack::isExhausted() const { return m_exhausted; }

} }
