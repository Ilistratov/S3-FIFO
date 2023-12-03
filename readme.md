# Single-threaded c++ implementatino of S3FIFO from https://github.com/Thesys-lab/sosp23-s3fifo

## How to run

0. Have python3+numpy and cpp compiler installed
1. edit `CC` in Makefile to match your compiler(g++ by default)
2. `make run` (should take ~3mins)
3. observe the cache-hit ratio.