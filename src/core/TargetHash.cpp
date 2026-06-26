#include "TargetHash.hpp"

#include <sstream>
#include <iomanip>

namespace fenrir {
namespace core {

TargetHash::TargetHash(HashType algo, std::vector<uint8_t> hashBytes,
                       std::vector<uint8_t> saltBytes)
    : m_algorithm(algo)
    , m_hash(std::move(hashBytes))
    , m_salt(std::move(saltBytes)) {}

HashType TargetHash::algorithm() const { return m_algorithm; }
const std::vector<uint8_t>& TargetHash::hash() const { return m_hash; }
const std::vector<uint8_t>& TargetHash::salt() const { return m_salt; }

std::string TargetHash::hex() const {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto b : m_hash) ss << std::setw(2) << static_cast<int>(b);
    return ss.str();
}

}
}
