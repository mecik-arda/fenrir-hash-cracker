__constant uint SHA1_K[4] = {
    0x5a827999,
    0x6ed9eba1,
    0x8f1bbcdc,
    0xca62c1d6
};

__kernel void sha1_crack(
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


    uint W[80];
    for (int i = 0; i < 16; i++) {
        W[i] = ((uint)msg[i*4] << 24) | ((uint)msg[i*4+1] << 16) |
               ((uint)msg[i*4+2] << 8) | (uint)msg[i*4+3];
    }


    for (int i = 16; i < 80; i++) {
        W[i] = ROTL32(W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16], 1);
    }


    uint a = 0x67452301;
    uint b = 0xefcdab89;
    uint c = 0x98badcfe;
    uint d = 0x10325476;
    uint e = 0xc3d2e1f0;


    #define SHA1_RND(t, k) \
    { \
        uint temp = ROTL32(a, 5) + SHA_CH(b, c, d) + e + k + W[t]; \
        e = d; d = c; c = ROTL32(b, 30); b = a; a = temp; \
    }


    SHA1_RND( 0, 0x5a827999); SHA1_RND( 1, 0x5a827999); SHA1_RND( 2, 0x5a827999);
    SHA1_RND( 3, 0x5a827999); SHA1_RND( 4, 0x5a827999); SHA1_RND( 5, 0x5a827999);
    SHA1_RND( 6, 0x5a827999); SHA1_RND( 7, 0x5a827999); SHA1_RND( 8, 0x5a827999);
    SHA1_RND( 9, 0x5a827999); SHA1_RND(10, 0x5a827999); SHA1_RND(11, 0x5a827999);
    SHA1_RND(12, 0x5a827999); SHA1_RND(13, 0x5a827999); SHA1_RND(14, 0x5a827999);
    SHA1_RND(15, 0x5a827999); SHA1_RND(16, 0x5a827999); SHA1_RND(17, 0x5a827999);
    SHA1_RND(18, 0x5a827999); SHA1_RND(19, 0x5a827999);


    #undef SHA1_RND
    #define SHA1_RND(t, k) \
    { \
        uint temp = ROTL32(a, 5) + SHA_PARITY(b, c, d) + e + k + W[t]; \
        e = d; d = c; c = ROTL32(b, 30); b = a; a = temp; \
    }

    SHA1_RND(20, 0x6ed9eba1); SHA1_RND(21, 0x6ed9eba1); SHA1_RND(22, 0x6ed9eba1);
    SHA1_RND(23, 0x6ed9eba1); SHA1_RND(24, 0x6ed9eba1); SHA1_RND(25, 0x6ed9eba1);
    SHA1_RND(26, 0x6ed9eba1); SHA1_RND(27, 0x6ed9eba1); SHA1_RND(28, 0x6ed9eba1);
    SHA1_RND(29, 0x6ed9eba1); SHA1_RND(30, 0x6ed9eba1); SHA1_RND(31, 0x6ed9eba1);
    SHA1_RND(32, 0x6ed9eba1); SHA1_RND(33, 0x6ed9eba1); SHA1_RND(34, 0x6ed9eba1);
    SHA1_RND(35, 0x6ed9eba1); SHA1_RND(36, 0x6ed9eba1); SHA1_RND(37, 0x6ed9eba1);
    SHA1_RND(38, 0x6ed9eba1); SHA1_RND(39, 0x6ed9eba1);


    #undef SHA1_RND
    #define SHA1_RND(t, k) \
    { \
        uint temp = ROTL32(a, 5) + SHA_MAJ(b, c, d) + e + k + W[t]; \
        e = d; d = c; c = ROTL32(b, 30); b = a; a = temp; \
    }

    SHA1_RND(40, 0x8f1bbcdc); SHA1_RND(41, 0x8f1bbcdc); SHA1_RND(42, 0x8f1bbcdc);
    SHA1_RND(43, 0x8f1bbcdc); SHA1_RND(44, 0x8f1bbcdc); SHA1_RND(45, 0x8f1bbcdc);
    SHA1_RND(46, 0x8f1bbcdc); SHA1_RND(47, 0x8f1bbcdc); SHA1_RND(48, 0x8f1bbcdc);
    SHA1_RND(49, 0x8f1bbcdc); SHA1_RND(50, 0x8f1bbcdc); SHA1_RND(51, 0x8f1bbcdc);
    SHA1_RND(52, 0x8f1bbcdc); SHA1_RND(53, 0x8f1bbcdc); SHA1_RND(54, 0x8f1bbcdc);
    SHA1_RND(55, 0x8f1bbcdc); SHA1_RND(56, 0x8f1bbcdc); SHA1_RND(57, 0x8f1bbcdc);
    SHA1_RND(58, 0x8f1bbcdc); SHA1_RND(59, 0x8f1bbcdc);


    #undef SHA1_RND
    #define SHA1_RND(t, k) \
    { \
        uint temp = ROTL32(a, 5) + SHA_PARITY(b, c, d) + e + k + W[t]; \
        e = d; d = c; c = ROTL32(b, 30); b = a; a = temp; \
    }

    SHA1_RND(60, 0xca62c1d6); SHA1_RND(61, 0xca62c1d6); SHA1_RND(62, 0xca62c1d6);
    SHA1_RND(63, 0xca62c1d6); SHA1_RND(64, 0xca62c1d6); SHA1_RND(65, 0xca62c1d6);
    SHA1_RND(66, 0xca62c1d6); SHA1_RND(67, 0xca62c1d6); SHA1_RND(68, 0xca62c1d6);
    SHA1_RND(69, 0xca62c1d6); SHA1_RND(70, 0xca62c1d6); SHA1_RND(71, 0xca62c1d6);
    SHA1_RND(72, 0xca62c1d6); SHA1_RND(73, 0xca62c1d6); SHA1_RND(74, 0xca62c1d6);
    SHA1_RND(75, 0xca62c1d6); SHA1_RND(76, 0xca62c1d6); SHA1_RND(77, 0xca62c1d6);
    SHA1_RND(78, 0xca62c1d6); SHA1_RND(79, 0xca62c1d6);

    #undef SHA1_RND


    a += 0x67452301;
    b += 0xefcdab89;
    c += 0x98badcfe;
    d += 0x10325476;
    e += 0xc3d2e1f0;



    if (a == target_prefix[0]) {
        results[idx * 5 + 0] = a;
        results[idx * 5 + 1] = b;
        results[idx * 5 + 2] = c;
        results[idx * 5 + 3] = d;
        results[idx * 5 + 4] = e;
        atom_min(found_flag, (int)idx);
    }
}
