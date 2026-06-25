#pragma once

#include <string>
#include <vector>
#include <optional>

namespace fenrir {
namespace api {

struct ApiQueryResult {
    bool found = false;
    std::string plaintext;
    std::string error;
};

class IApiProvider {
public:
    virtual ~IApiProvider() = default;


    virtual std::string name() const = 0;


    virtual std::optional<ApiQueryResult> query(const std::string& hash) = 0;


    virtual std::vector<ApiQueryResult> queryBatch(
        const std::vector<std::string>& hashes) = 0;


    virtual double maxRequestsPerSecond() const = 0;
};

}
}
