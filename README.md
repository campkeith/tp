# tp

`tp` is a multithreaded memory throughput measurement tool.

## Usage

`tp` is currently limited to read throughput only.

    $ make
    cc -std=c99 -Wall -Ofast -lpthread tp.c -o tp
    $ ./tp
    ./tp <num_threads> <size_bytes>
    $ ./tp 4 $((4 * 1024 * 1024 * 1024))
    50.739 GB/s

Recommended arguments:

* *num_threads*: number of CPU cores. For non-homogenous CPUs,
also try just the number of high-performance cores.

* *size_bytes*: around half of physical memory (minimizes CPU cache cheating)

## Internals

* Multithreaded (enables best chance of bottlenecking on DRAM)
* Buffer is initialized with random data (thwarts memory compression cheating).
* Time is measured with raw monotonic clock (thwarts Network Time Protocol fudging).
* 64-bit CPU memory accesses
* Read correctness check (simple checksum)
* Straight C/pthread code (no unnecessary/hidden complexity/dependencies)
