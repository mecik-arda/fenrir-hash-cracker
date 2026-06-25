__kernel void scrypt_crack(
    __global const uchar  *candidates,
    __global const uint   *offsets,
    __global const uchar  *lengths,
    __constant     uint   *salt,
    __constant     uint   N,
    __constant     uint   r,
    __constant     uint   p,
    __global       uint   *found_flag,
    __local        uchar  *scratchpad
) {
    uint idx = get_global_id(0);
    uint lid = get_local_id(0);
    uint lsize = get_local_size(0);







}
