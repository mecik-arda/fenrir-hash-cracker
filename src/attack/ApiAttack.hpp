#pragma once
#include "IAttackMode.hpp"
#include <memory>
#include <string>

namespace fenrir { namespace api { class ApiManager; } }

namespace fenrir { namespace attack {

class ApiAttack : public IAttackMode {
public:
    ApiAttack();
    ~ApiAttack() override;
    std::string name() const override { return "API"; }
    void initialize(const core::Config& cfg) override;
    bool nextBatch(std::vector<std::string>& candidates, size_t batchSize) override;
    std::vector<uint8_t> serializeState() const override;
    void deserializeState(const std::vector<uint8_t>& data) override;
    uint64_t totalCandidateEstimate() const override;
    uint64_t candidatesGenerated() const override;
    bool isExhausted() const override;

    std::unique_ptr<fenrir::api::ApiManager> m_apiManager;
    std::string m_apiKey;
    std::string m_provider;

private:
    uint64_t m_generated = 0;
    bool m_exhausted = false;
};

} }
