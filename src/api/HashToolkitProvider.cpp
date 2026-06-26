#include "HashToolkitProvider.hpp"
#include "../utils/Logger.hpp"

#ifdef FENRIR_HAS_API
#include <curl/curl.h>
#endif

namespace fenrir { namespace api {

static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

HashToolkitProvider::HashToolkitProvider(const std::string& apiKey) : m_apiKey(apiKey) {}

std::string HashToolkitProvider::httpGet(const std::string& url) {
#ifdef FENRIR_HAS_API
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        utils::Logger::warn("HashToolkit HTTP error: " + std::string(curl_easy_strerror(res)));
        curl_easy_cleanup(curl);
        return "";
    }
    curl_easy_cleanup(curl);
    return response;
#else
    utils::Logger::warn("libcurl not available — API mode disabled");
    return "";
#endif
}

std::optional<ApiQueryResult> HashToolkitProvider::query(const std::string& hash) {
    std::string url = "https://hashtoolkit.com/api/v2/hash/" + hash;
    auto response = httpGet(url);
    if (response.empty()) return std::nullopt;

    ApiQueryResult result;
    result.found = (response.find("plaintext") != std::string::npos &&
                    response.find("\"found\":true") != std::string::npos);

    if (result.found) {
        size_t pos = response.find("\"plaintext\":\"");
        if (pos != std::string::npos) {
            size_t start = pos + 13;
            size_t end = response.find('"', start);
            if (end != std::string::npos) {
                result.plaintext = response.substr(start, end - start);
            }
        }
    }
    return result;
}

std::vector<ApiQueryResult> HashToolkitProvider::queryBatch(const std::vector<std::string>& hashes) {
    std::vector<ApiQueryResult> results;
    for (const auto& hash : hashes) {
        auto r = query(hash);
        if (r) results.push_back(*r);
        else results.push_back({false, "", "Query failed"});
    }
    return results;
}

} }
