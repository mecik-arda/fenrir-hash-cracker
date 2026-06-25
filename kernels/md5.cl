__constant uint MD5_K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

__constant uint MD5_SHIFT[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

__kernel void md5_crack(
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
    if (pad_start < 64) {
        msg[pad_start] = 0x80;
    }


    for (uint i = pad_start + 1; i < 56; i++) {
        msg[i] = 0;
    }


    ulong bit_len = (ulong)len * 8;
    msg[56] = (uchar)(bit_len);
    msg[57] = (uchar)(bit_len >> 8);
    msg[58] = (uchar)(bit_len >> 16);
    msg[59] = (uchar)(bit_len >> 24);
    msg[60] = (uchar)(bit_len >> 32);
    msg[61] = (uchar)(bit_len >> 40);
    msg[62] = (uchar)(bit_len >> 48);
    msg[63] = (uchar)(bit_len >> 56);


    uint a = 0x67452301;
    uint b = 0xefcdab89;
    uint c = 0x98badcfe;
    uint d = 0x10325476;


    uint M[16];
    for (int i = 0; i < 16; i++) {
        M[i] = (uint)(msg[i*4]) | ((uint)(msg[i*4+1]) << 8) |
               ((uint)(msg[i*4+2]) << 16) | ((uint)(msg[i*4+3]) << 24);
    }


    #define MD5_STEP(fn, i, s, k, g) \
        a = b + ROTL32((a + fn(b, c, d) + M[g] + k), s)


    MD5_STEP(MD5_F,  0, 7,  0xd76aa478,  0);
    MD5_STEP(MD5_F,  1, 12, 0xe8c7b756,  1);
    MD5_STEP(MD5_F,  2, 17, 0x242070db,  2);
    MD5_STEP(MD5_F,  3, 22, 0xc1bdceee,  3);
    MD5_STEP(MD5_F,  4, 7,  0xf57c0faf,  4);
    MD5_STEP(MD5_F,  5, 12, 0x4787c62a,  5);
    MD5_STEP(MD5_F,  6, 17, 0xa8304613,  6);
    MD5_STEP(MD5_F,  7, 22, 0xfd469501,  7);
    MD5_STEP(MD5_F,  8, 7,  0x698098d8,  8);
    MD5_STEP(MD5_F,  9, 12, 0x8b44f7af,  9);
    MD5_STEP(MD5_F, 10, 17, 0xffff5bb1, 10);
    MD5_STEP(MD5_F, 11, 22, 0x895cd7be, 11);
    MD5_STEP(MD5_F, 12, 7,  0x6b901122, 12);
    MD5_STEP(MD5_F, 13, 12, 0xfd987193, 13);
    MD5_STEP(MD5_F, 14, 17, 0xa679438e, 14);
    MD5_STEP(MD5_F, 15, 22, 0x49b40821, 15);


    MD5_STEP(MD5_G, 16, 5,  0xf61e2562,  1);
    MD5_STEP(MD5_G, 17, 9,  0xc040b340,  6);
    MD5_STEP(MD5_G, 18, 14, 0x265e5a51, 11);
    MD5_STEP(MD5_G, 19, 20, 0xe9b6c7aa,  0);
    MD5_STEP(MD5_G, 20, 5,  0xd62f105d,  5);
    MD5_STEP(MD5_G, 21, 9,  0x02441453, 10);
    MD5_STEP(MD5_G, 22, 14, 0xd8a1e681, 15);
    MD5_STEP(MD5_G, 23, 20, 0xe7d3fbc8,  4);
    MD5_STEP(MD5_G, 24, 5,  0x21e1cde6,  9);
    MD5_STEP(MD5_G, 25, 9,  0xc33707d6, 14);
    MD5_STEP(MD5_G, 26, 14, 0xf4d50d87,  3);
    MD5_STEP(MD5_G, 27, 20, 0x455a14ed,  8);
    MD5_STEP(MD5_G, 28, 5,  0xa9e3e905, 13);
    MD5_STEP(MD5_G, 29, 9,  0xfcefa3f8,  2);
    MD5_STEP(MD5_G, 30, 14, 0x676f02d9,  7);
    MD5_STEP(MD5_G, 31, 20, 0x8d2a4c8a, 12);


    MD5_STEP(MD5_H, 32, 4,  0xfffa3942,  5);
    MD5_STEP(MD5_H, 33, 11, 0x8771f681,  8);
    MD5_STEP(MD5_H, 34, 16, 0x6d9d6122, 11);
    MD5_STEP(MD5_H, 35, 23, 0xfde5380c, 14);
    MD5_STEP(MD5_H, 36, 4,  0xa4beea44,  1);
    MD5_STEP(MD5_H, 37, 11, 0x4bdecfa9,  4);
    MD5_STEP(MD5_H, 38, 16, 0xf6bb4b60,  7);
    MD5_STEP(MD5_H, 39, 23, 0xbebfbc70, 10);
    MD5_STEP(MD5_H, 40, 4,  0x289b7ec6, 13);
    MD5_STEP(MD5_H, 41, 11, 0xeaa127fa,  0);
    MD5_STEP(MD5_H, 42, 16, 0xd4ef3085,  3);
    MD5_STEP(MD5_H, 43, 23, 0x04881d05,  6);
    MD5_STEP(MD5_H, 44, 4,  0xd9d4d039,  9);
    MD5_STEP(MD5_H, 45, 11, 0xe6db99e5, 12);
    MD5_STEP(MD5_H, 46, 16, 0x1fa27cf8, 15);
    MD5_STEP(MD5_H, 47, 23, 0xc4ac5665,  2);


    MD5_STEP(MD5_I, 48, 6,  0xf4292244,  0);
    MD5_STEP(MD5_I, 49, 10, 0x432aff97,  7);
    MD5_STEP(MD5_I, 50, 15, 0xab9423a7, 14);
    MD5_STEP(MD5_I, 51, 21, 0xfc93a039,  5);
    MD5_STEP(MD5_I, 52, 6,  0x655b59c3, 12);
    MD5_STEP(MD5_I, 53, 10, 0x8f0ccc92,  3);
    MD5_STEP(MD5_I, 54, 15, 0xffeff47d, 10);
    MD5_STEP(MD5_I, 55, 21, 0x85845dd1,  1);
    MD5_STEP(MD5_I, 56, 6,  0x6fa87e4f,  8);
    MD5_STEP(MD5_I, 57, 10, 0xfe2ce6e0, 15);
    MD5_STEP(MD5_I, 58, 15, 0xa3014314,  6);
    MD5_STEP(MD5_I, 59, 21, 0x4e0811a1, 13);
    MD5_STEP(MD5_I, 60, 6,  0xf7537e82,  4);
    MD5_STEP(MD5_I, 61, 10, 0xbd3af235, 11);
    MD5_STEP(MD5_I, 62, 15, 0x2ad7d2bb,  2);
    MD5_STEP(MD5_I, 63, 21, 0xeb86d391,  9);

    #undef MD5_STEP


    a += 0x67452301;
    b += 0xefcdab89;
    c += 0x98badcfe;
    d += 0x10325476;


    uint first_word = a;


    if (first_word == target_prefix[0]) {

        results[idx * 4 + 0] = a;
        results[idx * 4 + 1] = b;
        results[idx * 4 + 2] = c;
        results[idx * 4 + 3] = d;


        atom_min(found_flag, (int)idx);
    }
}
