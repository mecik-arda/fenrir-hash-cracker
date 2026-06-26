#include <catch2/catch_test_macros.hpp>
#include "../src/attack/MaskAttack.hpp"
#include "../src/core/Config.hpp"
#include <set>

using namespace fenrir::attack;
using namespace fenrir::core;

TEST_CASE("Mask ?l?l generates correct candidate count", "[mask]") {
    Config cfg;
    cfg.maskPattern = "?l?l";

    MaskAttack attack;
    attack.initialize(cfg);

    std::vector<std::string> candidates;
    attack.nextBatch(candidates, 2000);

    REQUIRE(candidates.size() == 26 * 26);
    REQUIRE(attack.isExhausted() == true);

    // Verify all candidates are unique
    std::set<std::string> unique_candidates(candidates.begin(), candidates.end());
    REQUIRE(unique_candidates.size() == 26 * 26);

    // Verify all candidates are exactly 2 chars and only lowercase
    for (const auto& c : candidates) {
        REQUIRE(c.size() == 2);
        bool allLower = true;
        for (char ch : c) {
            if (ch < 'a' || ch > 'z') { allLower = false; break; }
        }
        REQUIRE(allLower);
    }
}

TEST_CASE("Mask ?d?d?d generates 1000 candidates", "[mask]") {
    Config cfg;
    cfg.maskPattern = "?d?d?d";

    MaskAttack attack;
    attack.initialize(cfg);

    REQUIRE(attack.totalCandidateEstimate() == 1000);

    std::vector<std::string> candidates;
    attack.nextBatch(candidates, 2000);
    
    REQUIRE(candidates.size() == 1000);
    REQUIRE(attack.isExhausted() == true);
}
