__kernel void vecAdd(
    __global float *a,
    __global float *b,
    __global float *ab
) {
    int id = get_global_id(0);

    ab[id] = a[id] + b[id];
}
