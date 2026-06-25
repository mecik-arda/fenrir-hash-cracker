#include <catch2/catch_test_macros.hpp>
#include "../src/utils/HashParser.hpp"
#include "../src/core/TargetHash.hpp"

#include <sstream>
#include <iomanip>

using namespace fenrir::core;
using namespace fenrir::utils;

TEST_CASE("Detect MD5 from hex length", "[parser]") {
    REQUIRE(HashParser::detectType("5d41402abc4b2a76b9719d911017c592") == HashType::MD5);
}

TEST_CASE("Detect SHA1 from hex length", "[parser]") {
    REQUIRE(HashParser::detectType("da39a3ee5e6b4b0d3255bfef95601890afd80709") == HashType::SHA1);
}

TEST_CASE("Detect SHA256 from hex length", "[parser]") {
    REQUIRE(HashParser::detectType(
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855") == HashType::SHA256);
}

TEST_CASE("Detect SHA512 from hex length", "[parser]") {
    REQUIRE(HashParser::detectType(
        "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce"
        "47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e") == HashType::SHA512);
}

TEST_CASE("Detect bcrypt from $2a$ prefix", "[parser]") {
    REQUIRE(HashParser::detectType(
        "$2a$10$N9qo8uLOickgx2ZMRZoMyeIjZAgcfl7p92ldGxad68LJZdL17lhWy") == HashType::BCRYPT);
}

TEST_CASE("Detect scrypt from $7$ prefix", "[parser]") {
    REQUIRE(HashParser::detectType(
        "$7$C6..../....aaaaaaaaaaaaaaaaaaaaaa") == HashType::SCRYPT);
}

TEST_CASE("Parse hash:salt format", "[parser]") {
    auto target = HashParser::parse("5d41402abc4b2a76b9719d911017c592:salt123", HashType::MD5);
    REQUIRE(target.algorithm() == HashType::MD5);
    REQUIRE(target.salt().size() == 7);
    REQUIRE(std::string(target.salt().begin(), target.salt().end()) == "salt123");
}

TEST_CASE("Parse hash without salt", "[parser]") {
    auto target = HashParser::parse("5d41402abc4b2a76b9719d911017c592", HashType::MD5);
    REQUIRE(target.algorithm() == HashType::MD5);
    REQUIRE(target.salt().empty());
    REQUIRE(target.hex() == "5d41402abc4b2a76b9719d911017c592");
}
