#pragma once
#include "IApiProvider.hpp"
#include <string>

namespace fenrir { namespace api {

class LeakLookupProvider : public IApiProvider {
public:
    explicit LeakLookupProvider(const std::string& apiKey);

    std::string name() const override { return "LeakLookup"; }
    std::optional<ApiQueryResult> query(const std::string& hash) override;
    std::vector<ApiQueryResult> queryBatch(const std::vector<std::string>& hashes) override;
    double maxRequestsPerSecond() const override { return 0.5; }

private:
    std::string m_apiKey;
    std::string httpGet(const std::string& url);
};

} }
