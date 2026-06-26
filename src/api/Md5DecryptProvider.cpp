#include "Md5DecryptProvider.hpp"
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

Md5DecryptProvider::Md5DecryptProvider(const std::string& apiKey) : m_apiKey(apiKey) {}

std::string Md5DecryptProvider::httpGet(const std::string& url) {
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
        utils::Logger::warn("Md5Decrypt HTTP error: " + std::string(curl_easy_strerror(res)));
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

std::optional<ApiQueryResult> Md5DecryptProvider::query(const std::string& hash) {
    std::string type = "md5";
    if (hash.length() == 40) type = "sha1";
    else if (hash.length() == 64) type = "sha256";
    else if (hash.length() == 128) type = "sha512";

    std::string url = "https://md5decrypt.net/Api/api.php?hash=" + hash +
                      "&hash_type=" + type + "&code=" + m_apiKey;
    auto response = httpGet(url);
    if (response.empty()) return std::nullopt;

    ApiQueryResult result;
    // Response is typically plaintext directly or error string
    if (response.find("error") == std::string::npos &&
        response.find("Error") == std::string::npos &&
        !response.empty()) {
        result.found = true;
        result.plaintext = response;
    }
    return result;
}

std::vector<ApiQueryResult> Md5DecryptProvider::queryBatch(const std::vector<std::string>& hashes) {
    std::vector<ApiQueryResult> results;
    for (const auto& hash : hashes) {
        auto r = query(hash);
        if (r) results.push_back(*r);
        else results.push_back({false, "", "Query failed"});
    }
    return results;
}

} }
