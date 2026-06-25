__constant ulong SHA512_K[80] = {
    0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
    0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
    0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
    0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
    0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
    0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
    0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
    0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
    0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
    0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
    0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
    0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
    0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
    0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
    0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
    0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
    0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
    0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
    0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
    0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

__kernel void sha512_crack(
    __global const uchar  *candidates,
    __global const uint   *offsets,
    __global const uchar  *lengths,
    __global       ulong  *results,
    __constant     ulong  *target_prefix,
    __global       int    *found_flag
) {
    uint idx = get_global_id(0);
    uint off = offsets[idx];
    uint len = lengths[idx];


    uchar msg[128];
    for (uint i = 0; i < len && i < 111; i++) {
        msg[i] = candidates[off + i];
    }
    uint pad_start = (len < 111) ? len : 128;
    msg[pad_start] = 0x80;
    for (uint i = pad_start + 1; i < 112; i++) msg[i] = 0;


    ulong bit_len = (ulong)len * 8;
    msg[112] = 0; msg[113] = 0; msg[114] = 0; msg[115] = 0;
    msg[116] = 0; msg[117] = 0; msg[118] = 0; msg[119] = 0;
    msg[120] = (uchar)(bit_len >> 56); msg[121] = (uchar)(bit_len >> 48);
    msg[122] = (uchar)(bit_len >> 40); msg[123] = (uchar)(bit_len >> 32);
    msg[124] = (uchar)(bit_len >> 24); msg[125] = (uchar)(bit_len >> 16);
    msg[126] = (uchar)(bit_len >> 8);  msg[127] = (uchar)(bit_len);


    ulong W[80];
    for (int i = 0; i < 16; i++) {
        W[i] = ((ulong)msg[i*8] << 56)   | ((ulong)msg[i*8+1] << 48) |
               ((ulong)msg[i*8+2] << 40) | ((ulong)msg[i*8+3] << 32) |
               ((ulong)msg[i*8+4] << 24) | ((ulong)msg[i*8+5] << 16) |
               ((ulong)msg[i*8+6] << 8)  | (ulong)msg[i*8+7];
    }


    for (int i = 16; i < 80; i++) {
        W[i] = SHA512_sig1(W[i-2]) + W[i-7] + SHA512_sig0(W[i-15]) + W[i-16];
    }


    ulong a = 0x6a09e667f3bcc908, b = 0xbb67ae8584caa73b;
    ulong c = 0x3c6ef372fe94f82b, d = 0xa54ff53a5f1d36f1;
    ulong e = 0x510e527fade682d1, f = 0x9b05688c2b3e6c1f;
    ulong g = 0x1f83d9abfb41bd6b, h = 0x5be0cd19137e2179;


    #define SHA512_RND(i) { \
        ulong t1 = h + SHA512_SIG1(e) + SHA_CH(e, f, g) + SHA512_K[i] + W[i]; \
        ulong t2 = SHA512_SIG0(a) + SHA_MAJ(a, b, c); \
        h = g; g = f; f = e; e = d + t1; \
        d = c; c = b; b = a; a = t1 + t2; \
    }

    SHA512_RND(0);  SHA512_RND(1);  SHA512_RND(2);  SHA512_RND(3);
    SHA512_RND(4);  SHA512_RND(5);  SHA512_RND(6);  SHA512_RND(7);
    SHA512_RND(8);  SHA512_RND(9);  SHA512_RND(10); SHA512_RND(11);
    SHA512_RND(12); SHA512_RND(13); SHA512_RND(14); SHA512_RND(15);
    SHA512_RND(16); SHA512_RND(17); SHA512_RND(18); SHA512_RND(19);
    SHA512_RND(20); SHA512_RND(21); SHA512_RND(22); SHA512_RND(23);
    SHA512_RND(24); SHA512_RND(25); SHA512_RND(26); SHA512_RND(27);
    SHA512_RND(28); SHA512_RND(29); SHA512_RND(30); SHA512_RND(31);
    SHA512_RND(32); SHA512_RND(33); SHA512_RND(34); SHA512_RND(35);
    SHA512_RND(36); SHA512_RND(37); SHA512_RND(38); SHA512_RND(39);
    SHA512_RND(40); SHA512_RND(41); SHA512_RND(42); SHA512_RND(43);
    SHA512_RND(44); SHA512_RND(45); SHA512_RND(46); SHA512_RND(47);
    SHA512_RND(48); SHA512_RND(49); SHA512_RND(50); SHA512_RND(51);
    SHA512_RND(52); SHA512_RND(53); SHA512_RND(54); SHA512_RND(55);
    SHA512_RND(56); SHA512_RND(57); SHA512_RND(58); SHA512_RND(59);
    SHA512_RND(60); SHA512_RND(61); SHA512_RND(62); SHA512_RND(63);
    SHA512_RND(64); SHA512_RND(65); SHA512_RND(66); SHA512_RND(67);
    SHA512_RND(68); SHA512_RND(69); SHA512_RND(70); SHA512_RND(71);
    SHA512_RND(72); SHA512_RND(73); SHA512_RND(74); SHA512_RND(75);
    SHA512_RND(76); SHA512_RND(77); SHA512_RND(78); SHA512_RND(79);

    #undef SHA512_RND


    a += 0x6a09e667f3bcc908; b += 0xbb67ae8584caa73b;
    c += 0x3c6ef372fe94f82b; d += 0xa54ff53a5f1d36f1;
    e += 0x510e527fade682d1; f += 0x9b05688c2b3e6c1f;
    g += 0x1f83d9abfb41bd6b; h += 0x5be0cd19137e2179;


    if (a == target_prefix[0]) {
        results[idx * 8 + 0] = a; results[idx * 8 + 1] = b;
        results[idx * 8 + 2] = c; results[idx * 8 + 3] = d;
        results[idx * 8 + 4] = e; results[idx * 8 + 5] = f;
        results[idx * 8 + 6] = g; results[idx * 8 + 7] = h;
        atom_min(found_flag, (int)idx);
    }
}
