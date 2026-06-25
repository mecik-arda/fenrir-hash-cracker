#include "MaskAttack.hpp"
#include <cmath>
#include <algorithm>

namespace fenrir { namespace attack {

std::string MaskAttack::charsetFor(char token, const core::Config& cfg) {
    switch (token) {
        case 'l': return "abcdefghijklmnopqrstuvwxyz";
        case 'u': return "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        case 'd': return "0123456789";
        case 's': return "!@#$%^&*()-_=+[]{};:'\"\\|,.<>/?~ ";
        case 'a': return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{};:'\"\\|,.<>/?~ ";
        case 'h': return "0123456789abcdef";
        case 'H': return "0123456789ABCDEF";
        case 'b': {
            std::string s(256, '\0');
            for (int i=0;i<256;i++) s[i]=static_cast<char>(i);
            return s;
        }
        case '1': return cfg.customCharset1.empty() ? "0123456789" : cfg.customCharset1;
        case '2': return cfg.customCharset2.empty() ? "0123456789" : cfg.customCharset2;
        case '3': return cfg.customCharset3.empty() ? "0123456789" : cfg.customCharset3;
        case '4': return cfg.customCharset4.empty() ? "0123456789" : cfg.customCharset4;
        default: return "abcdefghijklmnopqrstuvwxyz";
    }
}

void MaskAttack::buildFromMask(const std::string& mask, const core::Config& cfg) {
    m_slots.clear();
    for (size_t i=0;i<mask.size();i++) {
        if (mask[i]=='?' && i+1<mask.size()) {
            std::string cs = charsetFor(mask[i+1], cfg);

            const std::string common = "aeiousnrltcdpmhbgfyvkwzxzjqxAEIOUSNRLTCDPMHBGFYVKWZXZJQX0123456789";
            std::string ordered;
            for (char c : common) if (cs.find(c)!=std::string::npos) ordered+=c;
            for (char c : cs) if (ordered.find(c)==std::string::npos) ordered+=c;
            m_slots.push_back({ordered, ordered.size(), 0});
            i++;
        } else {
            m_slots.push_back({std::string(1,mask[i]), 1, 0});
        }
    }
}

void MaskAttack::buildFromCharset(const std::string& charset, int minLen, int maxLen) {

    m_slots.clear();
    for (int i=0;i<minLen;i++) {
        m_slots.push_back({charset, charset.size(), 0});
    }
}

void MaskAttack::initialize(const core::Config& cfg) {
    if (!cfg.maskPattern.empty()) {
        buildFromMask(cfg.maskPattern, cfg);
    } else if (!cfg.charset.empty()) {
        buildFromCharset(cfg.charset, cfg.minLength, cfg.maxLength);
    } else {
        buildFromMask("?l?l?l?l?d?d", cfg);
    }

    m_total = 1;
    for (auto& s : m_slots) {
        m_total *= s.size;
    }
    m_generated = 0;
    m_exhausted = false;
}

std::string MaskAttack::current() const {
    std::string s(m_slots.size(), ' ');
    for (size_t i=0;i<m_slots.size();i++)
        s[i] = m_slots[i].chars[m_slots[i].current];
    return s;
}

bool MaskAttack::advance() {
    for (int i=static_cast<int>(m_slots.size())-1;i>=0;i--) {
        m_slots[i].current++;
        if (m_slots[i].current<m_slots[i].size) return true;
        m_slots[i].current=0;
    }
    return false;
}

bool MaskAttack::nextBatch(std::vector<std::string>& candidates, size_t batchSize) {
    candidates.clear();
    candidates.reserve(batchSize);
    for (size_t i=0;i<batchSize && !m_exhausted;i++) {
        candidates.push_back(current());
        m_exhausted = !advance();
        if (m_exhausted) break;
    }
    m_generated += candidates.size();
    return !candidates.empty();
}

std::vector<uint8_t> MaskAttack::serializeState() const {
    std::vector<uint8_t> d(16,0);
    auto w = [&](size_t o, uint64_t v){std::memcpy(d.data()+o,&v,8);};
    w(0, m_slots.size());
    w(8, m_generated);
    for (size_t i=0;i<m_slots.size();i++) {
        uint64_t cur = m_slots[i].current;
        auto* p = reinterpret_cast<const uint8_t*>(&cur);
        d.insert(d.end(), p, p+8);
    }
    return d;
}

void MaskAttack::deserializeState(const std::vector<uint8_t>& d) {
    if (d.size()<16) return;
    uint64_t nSlots, gen;
    std::memcpy(&nSlots,d.data(),8);
    std::memcpy(&gen,d.data()+8,8);
    m_generated=gen;
    m_exhausted=false;
    for (size_t i=0;i<std::min(nSlots,m_slots.size());i++) {
        uint64_t cur;
        std::memcpy(&cur,d.data()+16+i*8,8);
        m_slots[i].current=static_cast<size_t>(cur);
    }
}

uint64_t MaskAttack::totalCandidateEstimate() const { return m_total; }
uint64_t MaskAttack::candidatesGenerated() const { return m_generated; }
bool MaskAttack::isExhausted() const { return m_exhausted; }

} }
