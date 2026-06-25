#include "AttackRegistry.hpp"
#include "DictionaryAttack.hpp"
#include "RuleBasedAttack.hpp"
#include "MaskAttack.hpp"
#include "HybridAttack.hpp"
#include "ApiAttack.hpp"
#include <algorithm>

namespace fenrir { namespace attack {

std::unique_ptr<IAttackMode> AttackRegistry::create(const std::string& name) {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lower == "dict" || lower == "dictionary")
        return std::make_unique<DictionaryAttack>();
    if (lower == "rule" || lower == "rulebased" || lower == "rule-based")
        return std::make_unique<RuleBasedAttack>();
    if (lower == "mask" || lower == "bruteforce" || lower == "brute-force")
        return std::make_unique<MaskAttack>();
    if (lower == "hybrid")
        return std::make_unique<HybridAttack>();
    if (lower == "api")
        return std::make_unique<ApiAttack>();

    return nullptr;
}

} }
