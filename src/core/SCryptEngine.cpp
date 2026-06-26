#include "SCryptEngine.hpp"
#include "SHA256Engine.hpp"

#include <cstring>
#include <algorithm>

namespace fenrir { namespace core {

namespace {
void pbkdf2HmacSha256(const std::string& pass, const std::vector<uint8_t>& salt,
                      uint32_t iterations, uint8_t* out, size_t outLen) {
    SHA256Engine sha;
    constexpr size_t B = 64;
    constexpr size_t hLen = 32;

    auto hmac = [&](const std::string& key, const std::vector<uint8_t>& data) -> std::vector<uint8_t> {
        std::vector<uint8_t> kPad(B, 0);
        if (key.size() > B) {
            auto h = sha.hash(key);
            std::memcpy(kPad.data(), h.data(), std::min(h.size(), B));
        } else {
            std::memcpy(kPad.data(), key.data(), key.size());
        }
        std::vector<uint8_t> iPad(B), oPad(B);
        for (size_t i=0;i<B;i++) { iPad[i]=kPad[i]^0x36; oPad[i]=kPad[i]^0x5c; }
        std::vector<uint8_t> inner(B + data.size());
        std::memcpy(inner.data(), iPad.data(), B);
        std::memcpy(inner.data()+B, data.data(), data.size());
        auto hInner = sha.hash(std::string(inner.begin(), inner.end()));
        std::vector<uint8_t> outer(B + hInner.size());
        std::memcpy(outer.data(), oPad.data(), B);
        std::memcpy(outer.data()+B, hInner.data(), hInner.size());
        return sha.hash(std::string(outer.begin(), outer.end()));
    };

    // Multiple PBKDF2 blocks for outputs > hLen
    uint32_t blockCount = static_cast<uint32_t>((outLen + hLen - 1) / hLen);
    std::vector<uint8_t> t(outLen, 0);

    for (uint32_t block = 1; block <= blockCount; block++) {
        std::vector<uint8_t> s(salt.begin(), salt.end());
        s.push_back(static_cast<uint8_t>(block >> 24));
        s.push_back(static_cast<uint8_t>(block >> 16));
        s.push_back(static_cast<uint8_t>(block >> 8));
        s.push_back(static_cast<uint8_t>(block));

        std::vector<uint8_t> u = hmac(pass, s);
        std::vector<uint8_t> tBlock = u;
        for (uint32_t i=1;i<iterations;i++) {
            u = hmac(pass, u);
            for (size_t j=0;j<hLen;j++) tBlock[j]^=u[j];
        }

        size_t offset = (block - 1) * hLen;
        size_t copyLen = std::min(hLen, outLen - offset);
        std::memcpy(out + offset, tBlock.data(), copyLen);
    }
}

void salsa20_8(uint32_t B[16]) {
    auto R = [](uint32_t a, uint32_t b) { return (a<<b)|(a>>(32-b)); };
    uint32_t x[16];
    for (int i=0;i<16;i++) x[i]=B[i];
    for (int i=0;i<4;i++) {
        x[ 4]^=R(x[ 0]+x[12],7); x[ 8]^=R(x[ 4]+x[ 0],9);
        x[12]^=R(x[ 8]+x[ 4],13);x[ 0]^=R(x[12]+x[ 8],18);
        x[ 9]^=R(x[ 5]+x[ 1],7); x[13]^=R(x[ 9]+x[ 5],9);
        x[ 1]^=R(x[13]+x[ 9],13);x[ 5]^=R(x[ 1]+x[13],18);
        x[14]^=R(x[10]+x[ 6],7); x[ 2]^=R(x[14]+x[10],9);
        x[ 6]^=R(x[ 2]+x[14],13);x[10]^=R(x[ 6]+x[ 2],18);
        x[ 3]^=R(x[15]+x[11],7); x[ 7]^=R(x[ 3]+x[15],9);
        x[11]^=R(x[ 7]+x[ 3],13);x[15]^=R(x[11]+x[ 7],18);
        x[ 1]^=R(x[ 0]+x[ 3],7); x[ 2]^=R(x[ 1]+x[ 0],9);
        x[ 3]^=R(x[ 2]+x[ 1],13);x[ 0]^=R(x[ 3]+x[ 2],18);
        x[ 6]^=R(x[ 5]+x[ 4],7); x[ 7]^=R(x[ 6]+x[ 5],9);
        x[ 4]^=R(x[ 7]+x[ 6],13);x[ 5]^=R(x[ 4]+x[ 7],18);
        x[11]^=R(x[10]+x[ 9],7); x[ 8]^=R(x[11]+x[10],9);
        x[ 9]^=R(x[ 8]+x[11],13);x[10]^=R(x[ 9]+x[ 8],18);
        x[12]^=R(x[15]+x[14],7); x[13]^=R(x[12]+x[15],9);
        x[14]^=R(x[13]+x[12],13);x[15]^=R(x[14]+x[13],18);
    }
    for (int i=0;i<16;i++) B[i]+=x[i];
}

void blockmixSalsa8(uint32_t* B, uint32_t* Y, uint32_t r) {
    // Standard scrypt blockmix: B is 2*r*16 uint32s, Y is 2*r*32 uint32s temporary
    uint32_t X[16];
    // Copy last block to X
    for (uint32_t j=0;j<16;j++) X[j]=B[(2*r-1)*16+j];
    for (uint32_t i=0;i<2*r;i++) {
        for (uint32_t j=0;j<16;j++) X[j]^=B[i*16+j];
        salsa20_8(X);
        // Store in Y: each output block is 32 uint32s wide
        for (uint32_t j=0;j<16;j++) Y[i*32+j]=X[j];
    }
    // Permute back into B: even Y blocks to first half, odd to second half
    for (uint32_t i=0;i<r;i++) {
        for (uint32_t j=0;j<16;j++) {
            B[i*16+j] = Y[(2*i)*32+j];       // even-indexed Y block
            B[(i+r)*16+j] = Y[(2*i+1)*32+j]; // odd-indexed Y block
        }
    }
}

void scryptROMix(const uint8_t* B, uint8_t* out, uint32_t N, uint32_t r) {
    uint32_t* X = new uint32_t[32*r];
    uint32_t* V = new uint32_t[32*N*r];
    uint32_t* Y = new uint32_t[64*r];

    for (uint32_t i=0;i<32*r;i++)
        X[i] = ((uint32_t)B[i*4]) | ((uint32_t)B[i*4+1]<<8) | ((uint32_t)B[i*4+2]<<16) | ((uint32_t)B[i*4+3]<<24);

    for (uint32_t i=0;i<N;i++) {
        std::memcpy(V + i*32*r, X, 32*r*sizeof(uint32_t));
        blockmixSalsa8(X, Y, r);
    }
    for (uint32_t i=0;i<N;i++) {
        uint32_t j = X[(2*r-1)*16] & (N-1);
        for (uint32_t k=0;k<32*r;k++) X[k]^=V[j*32*r+k];
        blockmixSalsa8(X, Y, r);
    }
    for (uint32_t i=0;i<32*r;i++) {
        out[i*4]=X[i];out[i*4+1]=X[i]>>8;out[i*4+2]=X[i]>>16;out[i*4+3]=X[i]>>24;
    }
    delete[] X; delete[] V; delete[] Y;
}
}

std::vector<uint8_t> SCryptEngine::hash(const std::string& input, const std::vector<uint8_t>& salt) const {

    uint32_t N=1024, r=8, p=1;
    std::vector<uint8_t> B(128*r*p);
    pbkdf2HmacSha256(input, salt, 1, B.data(), B.size());
    std::vector<uint8_t> out(128*r);
    scryptROMix(B.data(), out.data(), N, r);
    std::vector<uint8_t> result(32);
    pbkdf2HmacSha256(input, out, 1, result.data(), 32);
    return result;
}

void SCryptEngine::hashBatch(const std::vector<std::string>& inputs, std::vector<std::vector<uint8_t>>& outputs, const std::vector<uint8_t>& salt) const {
    outputs.resize(inputs.size());
    for (size_t i=0;i<inputs.size();i++) outputs[i]=hash(inputs[i],salt);
}

} }
