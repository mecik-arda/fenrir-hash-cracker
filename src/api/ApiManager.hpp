#pragma once

#include "IApiProvider.hpp"
#include "RateLimiter.hpp"
#include <memory>
#include <string>
#include <vector>

namespace fenrir {
namespace api {

class ApiManager {
public:
    ApiManager();


    std::unique_ptr<IApiProvider> createProvider(const std::string& name,
                                                   const std::string& apiKey);


    std::vector<ApiQueryResult> queryWithFallback(
        const std::vector<std::string>& hashes,
        const std::string& primaryProvider,
        const std::string& apiKey,
        int maxRetries = 3,
        int timeoutSeconds = 10);
};

}
}
