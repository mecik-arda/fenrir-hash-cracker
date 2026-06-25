#include <catch2/catch_test_macros.hpp>
#include "../src/attack/MaskAttack.hpp"
#include "../src/core/Config.hpp"
#include <set>

using namespace fenrir::attack;
using namespace fenrir::core;

TEST_CASE("Mask ?l?l generates 26*26 candidates", "[mask]") {
    Config cfg;
    cfg.maskPattern = "?l?l";

    MaskAttack attack;
    attack.initialize(cfg);

    REQUIRE(attack.totalCandidateEstimate() == 26 * 26);

    std::vector<std::string> candidates;
    attack.nextBatch(candidates, 1000); // Should fit all 676 in one batch
    
    REQUIRE(candidates.size() == 676);
    REQUIRE(attack.isExhausted() == true);
    REQUIRE(candidates.front() == "aa"); // First common letter in the optimized common array might not be 'a', but 'a' is usually tested
    // To strictly verify all combinations exist:
    std::set<std::string> unique_candidates(candidates.begin(), candidates.end());
    REQUIRE(unique_candidates.size() == 676);
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
