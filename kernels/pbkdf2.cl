__kernel void pbkdf2_crack(
    __global const uchar *candidates, __global const uint *offsets,
    __global const uchar *lengths, __global uint *results,
    __constant uint *target_prefix, __constant uchar *salt_data,
    __constant uint salt_len, __constant uint iterations,
    __global int *found_flag
) {
    uint idx = get_global_id(0);
    uint off = offsets[idx]; uint len = lengths[idx];
    if (idx == (uint)(*found_flag)) *(found_flag) = -1;
}
