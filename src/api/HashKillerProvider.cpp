#include "HashKillerProvider.hpp"
#include "../utils/Logger.hpp"

#ifdef FENRIR_HAS_API
#include <curl/curl.h>
#endif

namespace fenrir { namespace api {

static size_t writeCb(void* c, size_t s, size_t n, std::string* o) {
    o->append(static_cast<char*>(c), s*n);
    return s*n;
}

HashKillerProvider::HashKillerProvider(const std::string& apiKey) : m_apiKey(apiKey) {}

std::string HashKillerProvider::httpGet(const std::string& url) {
#ifdef FENRIR_HAS_API
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    std::string r;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Fenrir/1.0");
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return r;
#else
    return "";
#endif
}

std::string HashKillerProvider::httpPost(const std::string& url, const std::string& data) {
#ifdef FENRIR_HAS_API
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    std::string r;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return r;
#else
    return "";
#endif
}

std::optional<ApiQueryResult> HashKillerProvider::query(const std::string& hash) {

    std::string url = "https://hashkiller.io/api/v2/query";
    std::string postData = "hash=" + hash + "&key=" + m_apiKey;

    auto response = httpPost(url, postData);
    if (response.empty()) return std::nullopt;

    ApiQueryResult result;
    result.found = (response.find("plaintext") != std::string::npos ||
                    response.find("\"found\":true") != std::string::npos);

    if (result.found) {
        size_t pos = response.find("\"plaintext\":\"");
        if (pos != std::string::npos) {
            pos += 13;
            size_t end = response.find("\"", pos);
            if (end != std::string::npos)
                result.plaintext = response.substr(pos, end - pos);
        }
    }
    return result;
}

std::vector<ApiQueryResult> HashKillerProvider::queryBatch(
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
