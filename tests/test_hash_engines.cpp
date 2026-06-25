#include <catch2/catch_test_macros.hpp>

#include "../src/core/MD5Engine.hpp"
#include "../src/core/SHA1Engine.hpp"
#include "../src/core/SHA256Engine.hpp"
#include "../src/core/SHA512Engine.hpp"
#include "../src/core/NTMLEngine.hpp"

#include <sstream>
#include <iomanip>

using namespace fenrir::core;

static std::string hex(const std::vector<uint8_t>& v) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto b : v) ss << std::setw(2) << static_cast<int>(b);
    return ss.str();
}

TEST_CASE("MD5 RFC 1321 test vectors", "[md5][cpu]") {
    MD5Engine md5;


    REQUIRE(hex(md5.hash("")) == "d41d8cd98f00b204e9800998ecf8427e");


    REQUIRE(hex(md5.hash("a")) == "0cc175b9c0f1b6a831c399e269772661");


    REQUIRE(hex(md5.hash("abc")) == "900150983cd24fb0d6963f7d28e17f72");


    REQUIRE(hex(md5.hash("message digest")) == "f96b697d7cb7938d525a2f31aaf161d0");


    REQUIRE(hex(md5.hash("1234567890")) == "7c12772809c1c0c3deda6103b10fdfa0");


    REQUIRE(md5.isSlowHash() == false);
    REQUIRE(md5.hashSize() == 16);
}

TEST_CASE("SHA1 FIPS 180-4 test vectors", "[sha1][cpu]") {
    SHA1Engine sha1;


    REQUIRE(hex(sha1.hash("")) == "da39a3ee5e6b4b0d3255bfef95601890afd80709");


    REQUIRE(hex(sha1.hash("abc")) == "a9993e364706816aba3e25717850c26c9cd0d89d");


    REQUIRE(hex(sha1.hash("The quick brown fox jumps over the lazy dog")) ==
        "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");

    REQUIRE(sha1.isSlowHash() == false);
    REQUIRE(sha1.hashSize() == 20);
}

TEST_CASE("SHA256 FIPS 180-4 test vectors", "[sha256][cpu]") {
    SHA256Engine sha256;


    REQUIRE(hex(sha256.hash("")) ==
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");


    REQUIRE(hex(sha256.hash("abc")) ==
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    REQUIRE(sha256.isSlowHash() == false);
    REQUIRE(sha256.hashSize() == 32);
}

TEST_CASE("SHA512 FIPS 180-4 test vectors", "[sha512][cpu]") {
    SHA512Engine sha512;


    REQUIRE(hex(sha512.hash("")) ==
        "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce"
        "47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");


    REQUIRE(hex(sha512.hash("abc")) ==
        "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
        "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");

    REQUIRE(sha512.isSlowHash() == false);
    REQUIRE(sha512.hashSize() == 64);
}

TEST_CASE("NTLM known vectors", "[ntlm][cpu]") {
    NTMLEngine ntlm;


    REQUIRE(hex(ntlm.hash("")) == "31d6cfe0d16ae931b73c59d7e0c089c0");


    REQUIRE(hex(ntlm.hash("password")) == "8846f7eaee8fb117ad06bdd830b7586c");


    REQUIRE(hex(ntlm.hash("hashcat")) == "b4b9b02e6f09a9bd760f388b67351e2b");

    REQUIRE(ntlm.isSlowHash() == false);
    REQUIRE(ntlm.hashSize() == 16);
    REQUIRE(ntlm.supportsSalt() == false);
}
