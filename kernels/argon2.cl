__kernel void argon2_crack(
    __global const uchar *candidates, __global const uint *offsets,
    __global const uchar *lengths, __global uint *results,
    __constant uint *target_prefix, __constant uchar *salt_data,
    __constant uint salt_len, __constant uint memory_kb,
    __constant uint iterations, __global int *found_flag,
    __local uchar *scratchpad
) {
    uint idx = get_global_id(0);
    if (idx == (uint)(*found_flag)) *(found_flag) = -1;
}
