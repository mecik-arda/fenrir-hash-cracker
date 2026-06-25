#pragma once
#include "IAttackMode.hpp"
#include <vector>
#include <string>

namespace fenrir { namespace attack {

class MaskAttack : public IAttackMode {
public:
    std::string name() const override { return "Mask"; }
    void initialize(const core::Config& cfg) override;
    bool nextBatch(std::vector<std::string>& candidates, size_t batchSize) override;
    std::vector<uint8_t> serializeState() const override;
    void deserializeState(const std::vector<uint8_t>& data) override;
    uint64_t totalCandidateEstimate() const override;
    uint64_t candidatesGenerated() const override;
    bool isExhausted() const override;

private:
    struct Slot { std::string chars; size_t size; size_t current = 0; };
    std::vector<Slot> m_slots;
    uint64_t m_total = 0;
    uint64_t m_generated = 0;
    bool m_exhausted = false;

    void buildFromMask(const std::string& mask, const core::Config& cfg);
    void buildFromCharset(const std::string& charset, int minLen, int maxLen);
    std::string current() const;
    bool advance();
    std::string charsetFor(char token, const core::Config& cfg);
};

} }
