#include "SHA512Engine.hpp"
#include <cstring>

namespace fenrir { namespace core {

namespace {
constexpr uint64_t K[80] = {
    0x428a2f98d728ae22,0x7137449123ef65cd,0xb5c0fbcfec4d3b2f,0xe9b5dba58189dbbc,
    0x3956c25bf348b538,0x59f111f1b605d019,0x923f82a4af194f9b,0xab1c5ed5da6d8118,
    0xd807aa98a3030242,0x12835b0145706fbe,0x243185be4ee4b28c,0x550c7dc3d5ffb4e2,
    0x72be5d74f27b896f,0x80deb1fe3b1696b1,0x9bdc06a725c71235,0xc19bf174cf692694,
    0xe49b69c19ef14ad2,0xefbe4786384f25e3,0x0fc19dc68b8cd5b5,0x240ca1cc77ac9c65,
    0x2de92c6f592b0275,0x4a7484aa6ea6e483,0x5cb0a9dcbd41fbd4,0x76f988da831153b5,
    0x983e5152ee66dfab,0xa831c66d2db43210,0xb00327c898fb213f,0xbf597fc7beef0ee4,
    0xc6e00bf33da88fc2,0xd5a79147930aa725,0x06ca6351e003826f,0x142929670a0e6e70,
    0x27b70a8546d22ffc,0x2e1b21385c26c926,0x4d2c6dfc5ac42aed,0x53380d139d95b3df,
    0x650a73548baf63de,0x766a0abb3c77b2a8,0x81c2c92e47edaee6,0x92722c851482353b,
    0xa2bfe8a14cf10364,0xa81a664bbc423001,0xc24b8b70d0f89791,0xc76c51a30654be30,
    0xd192e819d6ef5218,0xd69906245565a910,0xf40e35855771202a,0x106aa07032bbd1b8,
    0x19a4c116b8d2d0c8,0x1e376c085141ab53,0x2748774cdf8eeb99,0x34b0bcb5e19b48a8,
    0x391c0cb3c5c95a63,0x4ed8aa4ae3418acb,0x5b9cca4f7763e373,0x682e6ff3d6b2b8a3,
    0x748f82ee5defb2fc,0x78a5636f43172f60,0x84c87814a1f0ab72,0x8cc702081a6439ec,
    0x90befffa23631e28,0xa4506cebde82bde9,0xbef9a3f7b2c67915,0xc67178f2e372532b,
    0xca273eceea26619c,0xd186b8c721c0c207,0xeada7dd6cde0eb1e,0xf57d4f7fee6ed178,
    0x06f067aa72176fba,0x0a637dc5a2c898a6,0x113f9804bef90dae,0x1b710b35131c471b,
    0x28db77f523047d84,0x32caab7b40c72493,0x3c9ebe0a15c9bebc,0x431d67c49c100d4c,
    0x4cc5d4becb3e42b6,0x597f299cfc657e2a,0x5fcb6fab3ad6faec,0x6c44198c4a475817
};
inline uint64_t R(uint64_t x, uint64_t n) { return (x>>n)|(x<<(64-n)); }
inline uint64_t S0(uint64_t x) { return R(x,28)^R(x,34)^R(x,39); }
inline uint64_t S1(uint64_t x) { return R(x,14)^R(x,18)^R(x,41); }
inline uint64_t s0(uint64_t x) { return R(x,1)^R(x,8)^(x>>7); }
inline uint64_t s1(uint64_t x) { return R(x,19)^R(x,61)^(x>>6); }
inline uint64_t CH(uint64_t x,uint64_t y,uint64_t z) { return (x&y)^(~x&z); }
inline uint64_t MAJ(uint64_t x,uint64_t y,uint64_t z) { return (x&y)^(x&z)^(y&z); }

void sha512Transform(const uint8_t* msg, std::size_t msgLen, uint8_t* digest) {
    uint64_t bitLen = static_cast<uint64_t>(msgLen) * 8;
    std::size_t padLen = ((msgLen%128)<112) ? ((msgLen/128)+1)*128 : ((msgLen/128)+2)*128;

    std::vector<uint8_t> buf(padLen, 0);
    std::memcpy(buf.data(), msg, msgLen);
    buf[msgLen] = 0x80;

    for (int i=0;i<8;i++) buf[padLen-16+i] = 0;
    for (int i=7;i>=0;i--) buf[padLen-8+i] = static_cast<uint8_t>(bitLen>>((7-i)*8));

    uint64_t H[8] = {0x6a09e667f3bcc908,0xbb67ae8584caa73b,0x3c6ef372fe94f82b,0xa54ff53a5f1d36f1,
                      0x510e527fade682d1,0x9b05688c2b3e6c1f,0x1f83d9abfb41bd6b,0x5be0cd19137e2179};

    for (std::size_t blk=0; blk<padLen; blk+=128) {
        uint64_t W[80];
        for (int i=0;i<16;i++)
            W[i] = ((uint64_t)buf[blk+i*8]<<56)|((uint64_t)buf[blk+i*8+1]<<48)|((uint64_t)buf[blk+i*8+2]<<40)|((uint64_t)buf[blk+i*8+3]<<32)|
                   ((uint64_t)buf[blk+i*8+4]<<24)|((uint64_t)buf[blk+i*8+5]<<16)|((uint64_t)buf[blk+i*8+6]<<8)|(uint64_t)buf[blk+i*8+7];
        for (int i=16;i<80;i++) W[i]=s1(W[i-2])+W[i-7]+s0(W[i-15])+W[i-16];
        uint64_t a=H[0],b=H[1],c=H[2],d=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
        for (int t=0;t<80;t++) {
            uint64_t t1=h+S1(e)+CH(e,f,g)+K[t]+W[t];
            uint64_t t2=S0(a)+MAJ(a,b,c);
            h=g;g=f;f=e;e=d+t1;d=c;c=b;b=a;a=t1+t2;
        }
        H[0]+=a;H[1]+=b;H[2]+=c;H[3]+=d;H[4]+=e;H[5]+=f;H[6]+=g;H[7]+=h;
    }
    for (int i=0;i<8;i++) {
        digest[i*8]=H[i]>>56;digest[i*8+1]=H[i]>>48;digest[i*8+2]=H[i]>>40;digest[i*8+3]=H[i]>>32;
        digest[i*8+4]=H[i]>>24;digest[i*8+5]=H[i]>>16;digest[i*8+6]=H[i]>>8;digest[i*8+7]=H[i];
    }
}
}

std::vector<uint8_t> SHA512Engine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {
    std::string s = salt.empty() ? input : std::string(salt.begin(), salt.end()) + input;
    std::vector<uint8_t> r(64);
    sha512Transform(reinterpret_cast<const uint8_t*>(s.data()), s.size(), r.data());
    return r;
}

void SHA512Engine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (size_t i=0;i<inputs.size();i++) outputs[i]=hash(inputs[i],salt);
}

} }
