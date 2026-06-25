__constant ulong KECCAK_RC[24] = {
    0x0000000000000001UL,0x0000000000008082UL,0x800000000000808aUL,
    0x8000000080008000UL,0x000000000000808bUL,0x0000000080000001UL,
    0x8000000080008081UL,0x8000000000008009UL,0x000000000000008aUL,
    0x0000000000000088UL,0x0000000080008009UL,0x000000008000000aUL,
    0x000000008000808bUL,0x800000000000008bUL,0x8000000000008089UL,
    0x8000000000008003UL,0x8000000000008002UL,0x8000000000000080UL,
    0x000000000000800aUL,0x800000008000000aUL,0x8000000080008081UL,
    0x8000000000008080UL,0x0000000080000001UL,0x8000000080008008UL
};

__kernel void sha3_crack(
    __global const uchar *candidates, __global const uint *offsets,
    __global const uchar *lengths, __global uint *results,
    __constant uint *target_prefix, __global int *found_flag
) {
    uint idx = get_global_id(0);
    uint off = offsets[idx]; uint len = lengths[idx];
    ulong state[25] = {0};
    uint rateBytes = 136;
    for (uint i = 0; i < len && i < rateBytes; i++) {
        uint bi = i / 8, bj = i % 8;
        state[bi] |= ((ulong)candidates[off + i]) << (bj * 8);
    }
    uint padPos = (len < rateBytes) ? len : rateBytes;
    if (padPos < rateBytes) state[padPos/8] ^= ((ulong)0x06) << ((padPos%8)*8);
    state[(rateBytes-1)/8] ^= ((ulong)0x80) << (((rateBytes-1)%8)*8);

    for (int r = 0; r < 24; r++) {
        ulong C[5], D[5];
        for (int x = 0; x < 5; x++) C[x] = state[x] ^ state[5+x] ^ state[10+x] ^ state[15+x] ^ state[20+x];
        for (int x = 0; x < 5; x++) D[x] = C[(x+4)%5] ^ rotate(C[(x+1)%5], (ulong)1);
        for (int x = 0; x < 25; x++) state[x] ^= D[x%5];
        ulong B[25];
        for (int x = 0; x < 5; x++) for (int y = 0; y < 5; y++)
            B[y*5+((2*x+3*y)%5)] = rotate(state[x*5+y], (ulong)((x*5+y==0)?0:((x*5+y==1)?1:((x*5+y==2)?62:((x*5+y==3)?28:((x*5+y==4)?27:((x*5+y==5)?36:((x*5+y==6)?44:((x*5+y==7)?6:((x*5+y==8)?55:((x*5+y==9)?20:((x*5+y==10)?3:((x*5+y==11)?10:((x*5+y==12)?43:((x*5+y==13)?25:((x*5+y==14)?39:((x*5+y==15)?41:((x*5+y==16)?45:((x*5+y==17)?15:((x*5+y==18)?21:((x*5+y==19)?8:((x*5+y==20)?18:((x*5+y==21)?2:((x*5+y==22)?61:((x*5+y==23)?56:14)))))))))))))))))))))))));
        for (int x = 0; x < 5; x++) for (int y = 0; y < 5; y++)
            state[x*5+y] = B[x*5+y] ^ (~B[((x+1)%5)*5+y] & B[((x+2)%5)*5+y]);
        state[0] ^= KECCAK_RC[r];
    }
    if ((uint)(state[0] & 0xFFFFFFFF) == target_prefix[0]) {
        for (int i = 0; i < 8; i++) results[idx * 8 + i] = (uint)(state[i] & 0xFFFFFFFF);
        atom_min(found_flag, (int)idx);
    }
}
