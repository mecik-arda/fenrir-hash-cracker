#pragma once
#include "IApiProvider.hpp"
#include <string>

namespace fenrir { namespace api {

class HashKillerProvider : public IApiProvider {
public:
    explicit HashKillerProvider(const std::string& apiKey);

    std::string name() const override { return "HashKiller"; }
    std::optional<ApiQueryResult> query(const std::string& hash) override;
    std::vector<ApiQueryResult> queryBatch(const std::vector<std::string>& hashes) override;
    double maxRequestsPerSecond() const override { return 1.0; }

private:
    std::string m_apiKey;
    std::string httpGet(const std::string& url);
    std::string httpPost(const std::string& url, const std::string& data);
};

} }
