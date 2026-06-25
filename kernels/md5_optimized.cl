__constant uint MD5_K[64] = {
    0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
    0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,
    0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
    0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
    0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
    0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
    0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
    0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};

#define MD5_R1(a,b,c,d,k,s,i) a = b + rotate((a + ((b & c) | (~b & d)) + M[k] + MD5_K[i]), (uint)(s))
#define MD5_R2(a,b,c,d,k,s,i) a = b + rotate((a + ((b & d) | (c & ~d)) + M[k] + MD5_K[i]), (uint)(s))
#define MD5_R3(a,b,c,d,k,s,i) a = b + rotate((a + (b ^ c ^ d) + M[k] + MD5_K[i]), (uint)(s))
#define MD5_R4(a,b,c,d,k,s,i) a = b + rotate((a + (c ^ (b | ~d)) + M[k] + MD5_K[i]), (uint)(s))

__kernel void md5_optimized_crack(
    __global const uchar  *candidates,
    __global const uint   *offsets,
    __global const uchar  *lengths,
    __global       uint   *results,
    __constant     uint   *target_prefix,
    __global       int    *found_flag,
    __local        uint   *local_K
) {
    uint idx = get_global_id(0);
    uint lid = get_local_id(0);
    uint lsize = get_local_size(0);

    if (lid < 64) local_K[lid] = MD5_K[lid];
    barrier(CLK_LOCAL_MEM_FENCE);

    uint off = offsets[idx];
    uint len = lengths[idx];

    uchar msg[64] = {0};
    for (uint i = 0; i < len && i < 64; i++) msg[i] = candidates[off + i];
    msg[len] = 0x80;

    ulong bit_len = (ulong)len * 8;
    msg[56] = (uchar)(bit_len);       msg[57] = (uchar)(bit_len >> 8);
    msg[58] = (uchar)(bit_len >> 16); msg[59] = (uchar)(bit_len >> 24);
    msg[60] = (uchar)(bit_len >> 32); msg[61] = (uchar)(bit_len >> 40);
    msg[62] = (uchar)(bit_len >> 48); msg[63] = (uchar)(bit_len >> 56);

    uint M[16];
    #pragma unroll
    for (int i = 0; i < 16; i++) {
        M[i] = (uint)(msg[i*4]) | ((uint)(msg[i*4+1]) << 8) |
               ((uint)(msg[i*4+2]) << 16) | ((uint)(msg[i*4+3]) << 24);
    }

    uint a = 0x67452301, b = 0xefcdab89, c = 0x98badcfe, d = 0x10325476;

    MD5_R1(a,b,c,d, 0, 7, 0); MD5_R1(d,a,b,c, 1,12, 1); MD5_R1(c,d,a,b, 2,17, 2); MD5_R1(b,c,d,a, 3,22, 3);
    MD5_R1(a,b,c,d, 4, 7, 4); MD5_R1(d,a,b,c, 5,12, 5); MD5_R1(c,d,a,b, 6,17, 6); MD5_R1(b,c,d,a, 7,22, 7);
    MD5_R1(a,b,c,d, 8, 7, 8); MD5_R1(d,a,b,c, 9,12, 9); MD5_R1(c,d,a,b,10,17,10); MD5_R1(b,c,d,a,11,22,11);
    MD5_R1(a,b,c,d,12, 7,12); MD5_R1(d,a,b,c,13,12,13); MD5_R1(c,d,a,b,14,17,14); MD5_R1(b,c,d,a,15,22,15);

    MD5_R2(a,b,c,d, 1, 5,16); MD5_R2(d,a,b,c, 6, 9,17); MD5_R2(c,d,a,b,11,14,18); MD5_R2(b,c,d,a, 0,20,19);
    MD5_R2(a,b,c,d, 5, 5,20); MD5_R2(d,a,b,c,10, 9,21); MD5_R2(c,d,a,b,15,14,22); MD5_R2(b,c,d,a, 4,20,23);
    MD5_R2(a,b,c,d, 9, 5,24); MD5_R2(d,a,b,c,14, 9,25); MD5_R2(c,d,a,b, 3,14,26); MD5_R2(b,c,d,a, 8,20,27);
    MD5_R2(a,b,c,d,13, 5,28); MD5_R2(d,a,b,c, 2, 9,29); MD5_R2(c,d,a,b, 7,14,30); MD5_R2(b,c,d,a,12,20,31);

    MD5_R3(a,b,c,d, 5, 4,32); MD5_R3(d,a,b,c, 8,11,33); MD5_R3(c,d,a,b,11,16,34); MD5_R3(b,c,d,a,14,23,35);
    MD5_R3(a,b,c,d, 1, 4,36); MD5_R3(d,a,b,c, 4,11,37); MD5_R3(c,d,a,b, 7,16,38); MD5_R3(b,c,d,a,10,23,39);
    MD5_R3(a,b,c,d,13, 4,40); MD5_R3(d,a,b,c, 0,11,41); MD5_R3(c,d,a,b, 3,16,42); MD5_R3(b,c,d,a, 6,23,43);
    MD5_R3(a,b,c,d, 9, 4,44); MD5_R3(d,a,b,c,12,11,45); MD5_R3(c,d,a,b,15,16,46); MD5_R3(b,c,d,a, 2,23,47);

    MD5_R4(a,b,c,d, 0, 6,48); MD5_R4(d,a,b,c, 7,10,49); MD5_R4(c,d,a,b,14,15,50); MD5_R4(b,c,d,a, 5,21,51);
    MD5_R4(a,b,c,d,12, 6,52); MD5_R4(d,a,b,c, 3,10,53); MD5_R4(c,d,a,b,10,15,54); MD5_R4(b,c,d,a, 1,21,55);
    MD5_R4(a,b,c,d, 8, 6,56); MD5_R4(d,a,b,c,15,10,57); MD5_R4(c,d,a,b, 6,15,58); MD5_R4(b,c,d,a,13,21,59);
    MD5_R4(a,b,c,d, 4, 6,60); MD5_R4(d,a,b,c,11,10,61); MD5_R4(c,d,a,b, 2,15,62); MD5_R4(b,c,d,a, 9,21,63);

    a += 0x67452301; b += 0xefcdab89; c += 0x98badcfe; d += 0x10325476;

    if ((a & 0xFFFFFFFF) == target_prefix[0]) {
        results[idx * 4 + 0] = a; results[idx * 4 + 1] = b;
        results[idx * 4 + 2] = c; results[idx * 4 + 3] = d;
        atom_min(found_flag, (int)idx);
    }
}
