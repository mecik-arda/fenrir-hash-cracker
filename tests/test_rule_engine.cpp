#include <catch2/catch_test_macros.hpp>
#include "../src/rules/RuleEngine.hpp"
#include "../src/rules/RuleParser.hpp"

using namespace fenrir::rules;

TEST_CASE("Apply lowercase rule", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("l");
    REQUIRE(rule.size() > 0);

    auto result = engine.apply(rule, "PassWord");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "password");
}

TEST_CASE("Apply uppercase rule", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("u");
    
    auto result = engine.apply(rule, "PassWord");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "PASSWORD");
}

TEST_CASE("Apply append rule", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("$1");
    
    auto result = engine.apply(rule, "password");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "password1");
}

TEST_CASE("Apply prepend rule", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("^!");
    
    auto result = engine.apply(rule, "password");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "!password");
}

TEST_CASE("Apply combined rules", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("c $1"); // capitalize and append 1
    
    auto result = engine.apply(rule, "password");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "Password1");
}
