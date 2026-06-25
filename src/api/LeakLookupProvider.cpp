#include "LeakLookupProvider.hpp"
#include "../utils/Logger.hpp"

#include <string>
#include <sstream>

#ifdef FENRIR_HAS_API
#include <curl/curl.h>
#endif

namespace fenrir { namespace api {

static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append(static_cast<char*>(contents), total);
    return total;
}

LeakLookupProvider::LeakLookupProvider(const std::string& apiKey)
    : m_apiKey(apiKey) {}

std::string LeakLookupProvider::httpGet(const std::string& url) {
#ifdef FENRIR_HAS_API
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Fenrir/1.0");


    struct curl_slist* headers = nullptr;
    std::string authHeader = "X-API-Key: " + m_apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        utils::Logger::warn("HTTP request failed: " + std::string(curl_easy_strerror(res)));
        return "";
    }
    if (httpCode == 429) {
        utils::Logger::warn("Rate limited by LeakLookup API");
        return "";
    }
    if (httpCode != 200) {
        utils::Logger::warn("LeakLookup API returned HTTP " + std::to_string(httpCode));
        return "";
    }

    return response;
#else
    utils::Logger::warn("libcurl not available — API mode disabled");
    return "";
#endif
}

std::optional<ApiQueryResult> LeakLookupProvider::query(const std::string& hash) {
    std::string url = "https:
    auto response = httpGet(url);

    if (response.empty()) return std::nullopt;

    ApiQueryResult result;
    result.found = (response != "null" && !response.empty() &&
                    response.find("not found") == std::string::npos &&
                    response.find("\"found\":false") == std::string::npos);

    if (result.found) {

        size_t pos = response.find("\"plaintext\":\"");
        if (pos != std::string::npos) {
            pos += 13;
            size_t end = response.find("\"", pos);
            if (end != std::string::npos) {
                result.plaintext = response.substr(pos, end - pos);
            }
        } else {

            result.plaintext = response;

            if (!result.plaintext.empty() && result.plaintext.front() == '"')
                result.plaintext = result.plaintext.substr(1);
            if (!result.plaintext.empty() && result.plaintext.back() == '"')
                result.plaintext.pop_back();
        }
    }

    return result;
}

std::vector<ApiQueryResult> LeakLookupProvider::queryBatch(
    const std::vector<std::string>& hashes) {
    std::vector<ApiQueryResult> results;
    for (const auto& h : hashes) {
        auto r = query(h);
        if (r) results.push_back(*r);
        else results.push_back({false, "", "Request failed"});
    }
    return results;
}

} }
