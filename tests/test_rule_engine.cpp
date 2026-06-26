#include <catch2/catch_test_macros.hpp>
#include "../src/rules/RuleEngine.hpp"
#include "../src/rules/RuleParser.hpp"

using namespace fenrir::rules;

TEST_CASE("Apply lowercase rule", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("l");
    REQUIRE(rule.size() > 0);

    std::string result;
    bool success = engine.apply(rule, "PassWord", result);
    REQUIRE(success);
    REQUIRE(result == "password");
}

TEST_CASE("Apply uppercase rule", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("u");
    
    std::string result;
    bool success = engine.apply(rule, "PassWord", result);
    REQUIRE(success);
    REQUIRE(result == "PASSWORD");
}

TEST_CASE("Apply append rule", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("$1");
    
    std::string result;
    bool success = engine.apply(rule, "password", result);
    REQUIRE(success);
    REQUIRE(result == "password1");
}

TEST_CASE("Apply prepend rule", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("^!");
    
    std::string result;
    bool success = engine.apply(rule, "password", result);
    REQUIRE(success);
    REQUIRE(result == "!password");
}

TEST_CASE("Apply combined rules", "[rules]") {
    RuleEngine engine;
    auto rule = RuleParser::parseSingle("c $1"); // capitalize and append 1
    
    std::string result;
    bool success = engine.apply(rule, "password", result);
    REQUIRE(success);
    REQUIRE(result == "Password1");
}
