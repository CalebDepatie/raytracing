# Raytracing Experiments

This was originally created as part of a course for looking at the outputs and speed costs of distributed and path tracing, which was implemented sequentially.

I wanted to then compare a methods of parallelising the tracing, using both OpenMP and OpenCL.

## Performance Comparisons

The sequential methods do not use the GPU for any acceleration of the math.
Program was evaluated on a Ryzen 5 4600H, and GTX 1050 3GB.
The performance testing is not rigorous and was more intended to give a general idea of the differences in speed.

Path Tracing
Using 64 rays per pixel and a max depth of 8
Sequential: 8.868s

Distributed Tracing
Using an 8x8 grid and a max depth of 8
Sequential: 292.38s
