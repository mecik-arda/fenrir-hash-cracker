__constant uint SHA256_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

__kernel void sha256_crack(
    __global const uchar  *candidates,
    __global const uint   *offsets,
    __global const uchar  *lengths,
    __global       uint   *results,
    __constant     uint   *target_prefix,
    __global       int    *found_flag
) {
    uint idx = get_global_id(0);
    uint off = offsets[idx];
    uint len = lengths[idx];


    uchar msg[64];
    for (uint i = 0; i < len && i < 55; i++) {
        msg[i] = candidates[off + i];
    }
    uint pad_start = (len < 55) ? len : 64;
    msg[pad_start] = 0x80;
    for (uint i = pad_start + 1; i < 56; i++) msg[i] = 0;


    ulong bit_len = (ulong)len * 8;
    msg[56] = (uchar)(bit_len >> 56);
    msg[57] = (uchar)(bit_len >> 48);
    msg[58] = (uchar)(bit_len >> 40);
    msg[59] = (uchar)(bit_len >> 32);
    msg[60] = (uchar)(bit_len >> 24);
    msg[61] = (uchar)(bit_len >> 16);
    msg[62] = (uchar)(bit_len >> 8);
    msg[63] = (uchar)(bit_len);


    uint W[64];
    for (int i = 0; i < 16; i++) {
        W[i] = ((uint)msg[i*4] << 24) | ((uint)msg[i*4+1] << 16) |
               ((uint)msg[i*4+2] << 8) | (uint)msg[i*4+3];
    }


    for (int i = 16; i < 64; i++) {
        W[i] = SHA256_sig1(W[i-2]) + W[i-7] + SHA256_sig0(W[i-15]) + W[i-16];
    }


    uint a = 0x6a09e667, b = 0xbb67ae85, c = 0x3c6ef372, d = 0xa54ff53a;
    uint e = 0x510e527f, f = 0x9b05688c, g = 0x1f83d9ab, h = 0x5be0cd19;


    #define SHA256_RND(i) { \
        uint t1 = h + SHA256_SIG1(e) + SHA_CH(e, f, g) + SHA256_K[i] + W[i]; \
        uint t2 = SHA256_SIG0(a) + SHA_MAJ(a, b, c); \
        h = g; g = f; f = e; e = d + t1; \
        d = c; c = b; b = a; a = t1 + t2; \
    }

    SHA256_RND(0);  SHA256_RND(1);  SHA256_RND(2);  SHA256_RND(3);
    SHA256_RND(4);  SHA256_RND(5);  SHA256_RND(6);  SHA256_RND(7);
    SHA256_RND(8);  SHA256_RND(9);  SHA256_RND(10); SHA256_RND(11);
    SHA256_RND(12); SHA256_RND(13); SHA256_RND(14); SHA256_RND(15);
    SHA256_RND(16); SHA256_RND(17); SHA256_RND(18); SHA256_RND(19);
    SHA256_RND(20); SHA256_RND(21); SHA256_RND(22); SHA256_RND(23);
    SHA256_RND(24); SHA256_RND(25); SHA256_RND(26); SHA256_RND(27);
    SHA256_RND(28); SHA256_RND(29); SHA256_RND(30); SHA256_RND(31);
    SHA256_RND(32); SHA256_RND(33); SHA256_RND(34); SHA256_RND(35);
    SHA256_RND(36); SHA256_RND(37); SHA256_RND(38); SHA256_RND(39);
    SHA256_RND(40); SHA256_RND(41); SHA256_RND(42); SHA256_RND(43);
    SHA256_RND(44); SHA256_RND(45); SHA256_RND(46); SHA256_RND(47);
    SHA256_RND(48); SHA256_RND(49); SHA256_RND(50); SHA256_RND(51);
    SHA256_RND(52); SHA256_RND(53); SHA256_RND(54); SHA256_RND(55);
    SHA256_RND(56); SHA256_RND(57); SHA256_RND(58); SHA256_RND(59);
    SHA256_RND(60); SHA256_RND(61); SHA256_RND(62); SHA256_RND(63);

    #undef SHA256_RND


    a += 0x6a09e667; b += 0xbb67ae85; c += 0x3c6ef372; d += 0xa54ff53a;
    e += 0x510e527f; f += 0x9b05688c; g += 0x1f83d9ab; h += 0x5be0cd19;


    if (a == target_prefix[0]) {
        results[idx * 8 + 0] = a; results[idx * 8 + 1] = b;
        results[idx * 8 + 2] = c; results[idx * 8 + 3] = d;
        results[idx * 8 + 4] = e; results[idx * 8 + 5] = f;
        results[idx * 8 + 6] = g; results[idx * 8 + 7] = h;
        atom_min(found_flag, (int)idx);
    }
}
