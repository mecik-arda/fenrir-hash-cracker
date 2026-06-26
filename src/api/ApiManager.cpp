#include "ApiManager.hpp"
#include "LeakLookupProvider.hpp"
#include "HashKillerProvider.hpp"
#include "HashToolkitProvider.hpp"
#include "Md5DecryptProvider.hpp"
#include "../utils/Logger.hpp"

#include <algorithm>
#include <thread>

namespace fenrir { namespace api {

ApiManager::ApiManager() {}

std::unique_ptr<IApiProvider> ApiManager::createProvider(
    const std::string& name, const std::string& apiKey) {

    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lower == "leaklookup")
        return std::make_unique<LeakLookupProvider>(apiKey);
    if (lower == "hashkiller")
        return std::make_unique<HashKillerProvider>(apiKey);
    if (lower == "hashtoolkit")
        return std::make_unique<HashToolkitProvider>(apiKey);
    if (lower == "md5decrypt")
        return std::make_unique<Md5DecryptProvider>(apiKey);

    utils::Logger::error("Unknown API provider: " + name);
    return nullptr;
}

std::vector<ApiQueryResult> ApiManager::queryWithFallback(
    const std::vector<std::string>& hashes,
    const std::string& primaryProvider,
    const std::string& apiKey,
    int maxRetries,
    int timeoutSeconds) {

    auto provider = createProvider(primaryProvider, apiKey);
    if (!provider) return {};

    RateLimiter limiter(provider->maxRequestsPerSecond());
    std::vector<ApiQueryResult> results;

    for (const auto& hash : hashes) {
        for (int attempt = 0; attempt <= maxRetries; attempt++) {
            limiter.wait();

            auto result = provider->query(hash);
            if (result) {
                results.push_back(*result);
                if (result->found) {
                    utils::Logger::info("API hit: " + hash + " -> " + result->plaintext);
                }
                break;
            }

            if (attempt < maxRetries) {

                int delay = (1 << attempt);
                utils::Logger::warn("API request failed, retrying in " +
                                   std::to_string(delay) + "s...");
                std::this_thread::sleep_for(std::chrono::seconds(delay));
            } else {
                results.push_back({false, "", "Max retries exceeded"});
            }
        }
    }

    return results;
}

} }
