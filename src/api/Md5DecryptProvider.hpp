#pragma once
#include "IApiProvider.hpp"
#include <string>

namespace fenrir { namespace api {

class Md5DecryptProvider : public IApiProvider {
public:
    explicit Md5DecryptProvider(const std::string& apiKey);

    std::string name() const override { return "Md5Decrypt"; }
    std::optional<ApiQueryResult> query(const std::string& hash) override;
    std::vector<ApiQueryResult> queryBatch(const std::vector<std::string>& hashes) override;
    double maxRequestsPerSecond() const override { return 1.0; }

private:
    std::string m_apiKey;
    std::string httpGet(const std::string& url);
};

} }
