__constant uint NTLM_K[3][4] = {
    { 0, 0, 0, 0 },
    { 0x5a827999, 0x5a827999, 0x5a827999, 0x5a827999 },
    { 0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1 }
};

__constant uint NTLM_SHIFT[3][4] = {
    { 3, 7, 11, 19 },
    { 3, 5, 9, 13 },
    { 3, 9, 11, 15 }
};

__kernel void ntlm_crack(
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


    ushort utf16[128];
    for (uint i = 0; i < len && i < 128; i++) {
        utf16[i] = (ushort)candidates[off + i];
    }


    uchar msg[64];
    uint byteLen = len * 2;
    for (uint i = 0; i < byteLen && i < 55; i++) {
        msg[i] = (uchar)(utf16[i/2] >> ((i & 1) ? 8 : 0));
    }


    uint pad_start = (byteLen < 55) ? byteLen : 64;
    msg[pad_start] = 0x80;
    for (uint i = pad_start + 1; i < 56; i++) msg[i] = 0;


    ulong bit_len = (ulong)byteLen * 8;
    for (int i = 0; i < 8; i++) {
        msg[56 + i] = (uchar)(bit_len >> (i * 8));
    }


    uint M[16];
    for (int i = 0; i < 16; i++) {
        M[i] = (uint)msg[i*4] | ((uint)msg[i*4+1] << 8) |
               ((uint)msg[i*4+2] << 16) | ((uint)msg[i*4+3] << 24);
    }


    uint a = 0x67452301;
    uint b = 0xefcdab89;
    uint c = 0x98badcfe;
    uint d = 0x10325476;


    #define MD4_R1(a, b, c, d, k, s) \
        a = ROTL32(a + ((b & c) | (~b & d)) + M[k], s)

    MD4_R1(a, b, c, d,  0,  3);
    MD4_R1(d, a, b, c,  1,  7);
    MD4_R1(c, d, a, b,  2, 11);
    MD4_R1(b, c, d, a,  3, 19);
    MD4_R1(a, b, c, d,  4,  3);
    MD4_R1(d, a, b, c,  5,  7);
    MD4_R1(c, d, a, b,  6, 11);
    MD4_R1(b, c, d, a,  7, 19);
    MD4_R1(a, b, c, d,  8,  3);
    MD4_R1(d, a, b, c,  9,  7);
    MD4_R1(c, d, a, b, 10, 11);
    MD4_R1(b, c, d, a, 11, 19);
    MD4_R1(a, b, c, d, 12,  3);
    MD4_R1(d, a, b, c, 13,  7);
    MD4_R1(c, d, a, b, 14, 11);
    MD4_R1(b, c, d, a, 15, 19);


    #define MD4_R2(a, b, c, d, k, s) \
        a = ROTL32(a + ((b & c) | (b & d) | (c & d)) + M[k] + 0x5a827999, s)

    MD4_R2(a, b, c, d,  0,  3);
    MD4_R2(d, a, b, c,  4,  5);
    MD4_R2(c, d, a, b,  8,  9);
    MD4_R2(b, c, d, a, 12, 13);
    MD4_R2(a, b, c, d,  1,  3);
    MD4_R2(d, a, b, c,  5,  5);
    MD4_R2(c, d, a, b,  9,  9);
    MD4_R2(b, c, d, a, 13, 13);
    MD4_R2(a, b, c, d,  2,  3);
    MD4_R2(d, a, b, c,  6,  5);
    MD4_R2(c, d, a, b, 10,  9);
    MD4_R2(b, c, d, a, 14, 13);
    MD4_R2(a, b, c, d,  3,  3);
    MD4_R2(d, a, b, c,  7,  5);
    MD4_R2(c, d, a, b, 11,  9);
    MD4_R2(b, c, d, a, 15, 13);


    #define MD4_R3(a, b, c, d, k, s) \
        a = ROTL32(a + (b ^ c ^ d) + M[k] + 0x6ed9eba1, s)

    MD4_R3(a, b, c, d,  0,  3);
    MD4_R3(d, a, b, c,  8,  9);
    MD4_R3(c, d, a, b,  4, 11);
    MD4_R3(b, c, d, a, 12, 15);
    MD4_R3(a, b, c, d,  2,  3);
    MD4_R3(d, a, b, c, 10,  9);
    MD4_R3(c, d, a, b,  6, 11);
    MD4_R3(b, c, d, a, 14, 15);
    MD4_R3(a, b, c, d,  1,  3);
    MD4_R3(d, a, b, c,  9,  9);
    MD4_R3(c, d, a, b,  5, 11);
    MD4_R3(b, c, d, a, 13, 15);
    MD4_R3(a, b, c, d,  3,  3);
    MD4_R3(d, a, b, c, 11,  9);
    MD4_R3(c, d, a, b,  7, 11);
    MD4_R3(b, c, d, a, 15, 15);

    #undef MD4_R1
    #undef MD4_R2
    #undef MD4_R3


    a += 0x67452301;
    b += 0xefcdab89;
    c += 0x98badcfe;
    d += 0x10325476;


    if (a == target_prefix[0]) {
        results[idx * 4 + 0] = a;
        results[idx * 4 + 1] = b;
        results[idx * 4 + 2] = c;
        results[idx * 4 + 3] = d;
        atom_min(found_flag, (int)idx);
    }
}
