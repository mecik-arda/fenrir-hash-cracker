#include "SHA256Engine.hpp"
#include <cstring>

namespace fenrir { namespace core {

namespace {
constexpr uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};
inline uint32_t R(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
inline uint32_t S0(uint32_t x) { return R(x,2)^R(x,13)^R(x,22); }
inline uint32_t S1(uint32_t x) { return R(x,6)^R(x,11)^R(x,25); }
inline uint32_t s0(uint32_t x) { return R(x,7)^R(x,18)^(x>>3); }
inline uint32_t s1(uint32_t x) { return R(x,17)^R(x,19)^(x>>10); }
inline uint32_t CH(uint32_t x,uint32_t y,uint32_t z) { return (x&y)^(~x&z); }
inline uint32_t MAJ(uint32_t x,uint32_t y,uint32_t z) { return (x&y)^(x&z)^(y&z); }

void sha256Transform(const uint8_t* msg, std::size_t msgLen, uint8_t* digest) {
    uint64_t bitLen = static_cast<uint64_t>(msgLen) * 8;
    std::size_t padLen = ((msgLen % 64) < 56) ? ((msgLen/64)+1)*64 : ((msgLen/64)+2)*64;

    std::vector<uint8_t> buf(padLen, 0);
    std::memcpy(buf.data(), msg, msgLen);
    buf[msgLen] = 0x80;
    for (int i = 7; i >= 0; i--) buf[padLen-8+i] = static_cast<uint8_t>(bitLen >> ((7-i)*8));

    uint32_t H[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};

    for (std::size_t blk = 0; blk < padLen; blk += 64) {
        uint32_t W[64];
        for (int i = 0; i < 16; i++)
            W[i] = ((uint32_t)buf[blk+i*4]<<24)|((uint32_t)buf[blk+i*4+1]<<16)|((uint32_t)buf[blk+i*4+2]<<8)|(uint32_t)buf[blk+i*4+3];
        for (int i = 16; i < 64; i++) W[i] = s1(W[i-2])+W[i-7]+s0(W[i-15])+W[i-16];

        uint32_t a=H[0],b=H[1],c=H[2],d=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
        for (int t = 0; t < 64; t++) {
            uint32_t t1 = h + S1(e) + CH(e,f,g) + K[t] + W[t];
            uint32_t t2 = S0(a) + MAJ(a,b,c);
            h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
        }
        H[0]+=a;H[1]+=b;H[2]+=c;H[3]+=d;H[4]+=e;H[5]+=f;H[6]+=g;H[7]+=h;
    }
    for (int i = 0; i < 8; i++) { digest[i*4]=H[i]>>24; digest[i*4+1]=H[i]>>16; digest[i*4+2]=H[i]>>8; digest[i*4+3]=H[i]; }
}
}

std::vector<uint8_t> SHA256Engine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    std::string s = salt.empty() ? input : std::string(salt.begin(), salt.end()) + input;
    std::vector<uint8_t> r(32);
    sha256Transform(reinterpret_cast<const uint8_t*>(s.data()), s.size(), r.data());
    return r;
}

void SHA256Engine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (size_t i=0;i<inputs.size();i++) outputs[i]=hash(inputs[i],salt);
}

} }
