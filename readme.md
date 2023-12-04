# Single-threaded C++ implementation of S3FIFO from [FIFO Queues are All You Need for Cache Eviction](https://dl.acm.org/doi/pdf/10.1145/3600006.3613147)

## How to run

0. Have python3+numpy and cpp compiler installed
1. edit `CC` in Makefile to match your compiler(g++ by default)
2. `make run` (should take ~3mins)
3. observe the cache-hit ratio.