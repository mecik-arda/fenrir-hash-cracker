#include "ApiAttack.hpp"
#ifdef FENRIR_HAS_API
#include "../api/ApiManager.hpp"
#else
namespace fenrir { namespace api {
    struct ApiQueryResult { bool found = false; std::string plaintext = ""; };
    struct ApiManager {
        std::vector<ApiQueryResult> queryWithFallback(const std::vector<std::string>&, const std::string&, const std::string&) { return {}; }
    };
}}
#endif
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
    
    // For simplicity in this demo, we read inlineHashes.
    m_targetHashes = cfg.inlineHashes;
    m_hashIndex = 0;
    if (m_targetHashes.empty()) {
        m_exhausted = true;
    }
}

bool ApiAttack::nextBatch(std::vector<std::string>& candidates, size_t batchSize) {
    candidates.clear();
    if (m_exhausted) return false;

    while (candidates.empty() && !m_exhausted) {
        std::vector<std::string> batchHashes;
        while (batchHashes.size() < batchSize && m_hashIndex < m_targetHashes.size()) {
            batchHashes.push_back(m_targetHashes[m_hashIndex++]);
        }

        if (batchHashes.empty()) {
            m_exhausted = true;
            return false;
        }

        auto results = m_apiManager->queryWithFallback(batchHashes, m_provider, m_apiKey);
        for (const auto& res : results) {
            if (res.found && !res.plaintext.empty()) {
                candidates.push_back(res.plaintext);
            }
        }
        
        if (m_hashIndex >= m_targetHashes.size()) {
            m_exhausted = true;
        }
    }

    m_generated += candidates.size();
    return !candidates.empty();
}

std::vector<uint8_t> ApiAttack::serializeState() const {
    std::vector<uint8_t> d(16,0);
    std::memcpy(d.data(), &m_generated, 8);
    uint64_t idx = m_hashIndex;
    std::memcpy(d.data()+8, &idx, 8);
    return d;
}

void ApiAttack::deserializeState(const std::vector<uint8_t>& d) {
    if (d.size()>=16) {
        std::memcpy(&m_generated, d.data(), 8);
        uint64_t idx;
        std::memcpy(&idx, d.data()+8, 8);
        m_hashIndex = static_cast<size_t>(idx);
    }
    m_exhausted = (m_hashIndex >= m_targetHashes.size());
}

uint64_t ApiAttack::totalCandidateEstimate() const { return 0; }
uint64_t ApiAttack::candidatesGenerated() const { return m_generated; }
bool ApiAttack::isExhausted() const { return m_exhausted; }

} }
