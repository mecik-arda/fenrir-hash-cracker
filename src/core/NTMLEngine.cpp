#include "NTMLEngine.hpp"
#include <cstring>
#include <cwchar>

namespace fenrir { namespace core {

namespace {
inline uint32_t ROTL32(uint32_t x, uint32_t n) { return (x << n) | (x >> (32 - n)); }

void md4Transform(const uint8_t* msg, std::size_t msgLen, uint8_t* digest) {
    std::size_t padLen = ((msgLen % 64) < 56) ? ((msgLen/64)+1)*64 : ((msgLen/64)+2)*64;

    std::vector<uint8_t> buf(padLen, 0);
    std::memcpy(buf.data(), msg, msgLen);
    buf[msgLen] = 0x80;
    uint64_t bitLen = static_cast<uint64_t>(msgLen) * 8;
    for (int i=0;i<8;i++) buf[padLen-8+i] = static_cast<uint8_t>(bitLen >> (i*8));

    uint32_t a=0x67452301, b=0xefcdab89, c=0x98badcfe, d=0x10325476;

    for (std::size_t blk=0; blk<padLen; blk+=64) {
        uint32_t M[16];
        for (int i=0;i<16;i++)
            M[i] = (uint32_t)buf[blk+i*4] | ((uint32_t)buf[blk+i*4+1]<<8) |
                   ((uint32_t)buf[blk+i*4+2]<<16) | ((uint32_t)buf[blk+i*4+3]<<24);
        uint32_t aa=a, bb=b, cc=c, dd=d;


        auto R1 = [](uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t k, uint32_t s) {
            a = ROTL32(a + ((b & c) | (~b & d)) + k, s);
        };
        R1(a,b,c,d,M[ 0], 3); R1(d,a,b,c,M[ 1], 7); R1(c,d,a,b,M[ 2],11); R1(b,c,d,a,M[ 3],19);
        R1(a,b,c,d,M[ 4], 3); R1(d,a,b,c,M[ 5], 7); R1(c,d,a,b,M[ 6],11); R1(b,c,d,a,M[ 7],19);
        R1(a,b,c,d,M[ 8], 3); R1(d,a,b,c,M[ 9], 7); R1(c,d,a,b,M[10],11); R1(b,c,d,a,M[11],19);
        R1(a,b,c,d,M[12], 3); R1(d,a,b,c,M[13], 7); R1(c,d,a,b,M[14],11); R1(b,c,d,a,M[15],19);


        auto R2 = [](uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t k, uint32_t s) {
            a = ROTL32(a + ((b & c) | (b & d) | (c & d)) + k + 0x5a827999, s);
        };
        R2(a,b,c,d,M[ 0], 3); R2(d,a,b,c,M[ 4], 5); R2(c,d,a,b,M[ 8], 9); R2(b,c,d,a,M[12],13);
        R2(a,b,c,d,M[ 1], 3); R2(d,a,b,c,M[ 5], 5); R2(c,d,a,b,M[ 9], 9); R2(b,c,d,a,M[13],13);
        R2(a,b,c,d,M[ 2], 3); R2(d,a,b,c,M[ 6], 5); R2(c,d,a,b,M[10], 9); R2(b,c,d,a,M[14],13);
        R2(a,b,c,d,M[ 3], 3); R2(d,a,b,c,M[ 7], 5); R2(c,d,a,b,M[11], 9); R2(b,c,d,a,M[15],13);


        auto R3 = [](uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t k, uint32_t s) {
            a = ROTL32(a + (b ^ c ^ d) + k + 0x6ed9eba1, s);
        };
        R3(a,b,c,d,M[ 0], 3); R3(d,a,b,c,M[ 8], 9); R3(c,d,a,b,M[ 4],11); R3(b,c,d,a,M[12],15);
        R3(a,b,c,d,M[ 2], 3); R3(d,a,b,c,M[10], 9); R3(c,d,a,b,M[ 6],11); R3(b,c,d,a,M[14],15);
        R3(a,b,c,d,M[ 1], 3); R3(d,a,b,c,M[ 9], 9); R3(c,d,a,b,M[ 5],11); R3(b,c,d,a,M[13],15);
        R3(a,b,c,d,M[ 3], 3); R3(d,a,b,c,M[11], 9); R3(c,d,a,b,M[ 7],11); R3(b,c,d,a,M[15],15);

        a+=aa; b+=bb; c+=cc; d+=dd;
    }
    auto w32 = [](uint8_t* o, uint32_t v){o[0]=v;o[1]=v>>8;o[2]=v>>16;o[3]=v>>24;};
    w32(digest,a); w32(digest+4,b); w32(digest+8,c); w32(digest+12,d);
}
}

std::vector<uint8_t> NTMLEngine::hash(const std::string& input, const std::vector<uint8_t>&) const {

    std::vector<uint8_t> utf16;
    utf16.reserve(input.size() * 2);
    for (char ch : input) {
        uint16_t w = static_cast<uint16_t>(static_cast<uint8_t>(ch));
        utf16.push_back(static_cast<uint8_t>(w & 0xFF));
        utf16.push_back(static_cast<uint8_t>(w >> 8));
    }
    std::vector<uint8_t> r(16);
    md4Transform(utf16.data(), utf16.size(), r.data());
    return r;
}

void NTMLEngine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>&) const {
    outputs.resize(inputs.size());
    for (size_t i=0;i<inputs.size();i++) outputs[i]=hash(inputs[i]);
}

} }
