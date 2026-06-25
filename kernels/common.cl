#ifndef FENRIR_COMMON_CL
#define FENRIR_COMMON_CL

#define ROTL32(x, n) rotate((x), (uint)(n))
#define ROTR32(x, n) rotate((x), (uint)(32 - (n)))
#define ROTL64(x, n) rotate((x), (ulong)(n))
#define ROTR64(x, n) rotate((x), (ulong)(64 - (n)))

#define SWAP32(x) (as_uint(as_uchar4(x).wzyx))
#define SWAP64(x) (as_ulong(as_uchar8(x).s76543210))

#define MD5_F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | ~(z)))

#define SHA_CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define SHA_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA_PARITY(x, y, z) ((x) ^ (y) ^ (z))

#define SHA256_SIG0(x) (ROTR32(x, 2) ^ ROTR32(x, 13) ^ ROTR32(x, 22))
#define SHA256_SIG1(x) (ROTR32(x, 6) ^ ROTR32(x, 11) ^ ROTR32(x, 25))
#define SHA256_sig0(x) (ROTR32(x, 7) ^ ROTR32(x, 18) ^ ((x) >> 3))
#define SHA256_sig1(x) (ROTR32(x, 17) ^ ROTR32(x, 19) ^ ((x) >> 10))

#define SHA512_SIG0(x) (ROTR64(x, 28) ^ ROTR64(x, 34) ^ ROTR64(x, 39))
#define SHA512_SIG1(x) (ROTR64(x, 14) ^ ROTR64(x, 18) ^ ROTR64(x, 41))
#define SHA512_sig0(x) (ROTR64(x, 1) ^ ROTR64(x, 63) ^ ((x) >> 7))
#define SHA512_sig1(x) (ROTR64(x, 19) ^ ROTR64(x, 61) ^ ((x) >> 6))

inline void md5_sha_padding_64(uchar* block, uint msgLen, uint blockIdx, uint totalBlocks) {

    if (blockIdx < totalBlocks - 1) return;

    uint rem = msgLen & 0x3F;
    if (rem < 56) {

        if (rem < 64) {
            block[rem] = 0x80;
            for (uint i = rem + 1; i < 56; i++) block[i] = 0;
        }

    } else {

        block[rem] = 0x80;
        for (uint i = rem + 1; i < 64; i++) block[i] = 0;
    }
}

#endif
