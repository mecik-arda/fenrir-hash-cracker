#include "ApiAttack.hpp"
#include "../api/ApiManager.hpp"
#include "../utils/Logger.hpp"

namespace fenrir { namespace attack {

ApiAttack::ApiAttack() = default;
ApiAttack::~ApiAttack() = default;

void ApiAttack::initialize(const core::Config& cfg) {
    m_apiKey = cfg.apiKey;
    m_provider = cfg.apiProvider;
    m_apiManager = std::make_unique<api::ApiManager>();
    m_generated = 0;
    m_exhausted = false;

}

bool ApiAttack::nextBatch(std::vector<std::string>& candidates, size_t batchSize) {
    candidates.clear();
    if (m_exhausted) return false;






    m_exhausted = true;

    candidates.push_back("__API_LOOKUP__");
    return true;
}

std::vector<uint8_t> ApiAttack::serializeState() const {
    std::vector<uint8_t> d(8,0);
    std::memcpy(d.data(), &m_generated, 8);
    return d;
}

void ApiAttack::deserializeState(const std::vector<uint8_t>& d) {
    if (d.size()>=8) std::memcpy(&m_generated, d.data(), 8);
    m_exhausted = false;
}

uint64_t ApiAttack::totalCandidateEstimate() const { return 0; }
uint64_t ApiAttack::candidatesGenerated() const { return m_generated; }
bool ApiAttack::isExhausted() const { return m_exhausted; }

} }
