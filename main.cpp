#include "s3-fifo.h"
#include <iostream>
#include <stdint.h>
#include <unordered_map>

template <uint32_t Capacity> class MainStorage {
  uint64_t data_[Capacity];

public:
  uint32_t n_writes_ = 0;
  uint32_t n_reads_ = 0;

  void Write(uint32_t key, uint64_t value) {
    assert(key < Capacity);
    data_[key] = value;
    ++n_writes_;
  }

  uint64_t Read(uint32_t key) {
    assert(key < Capacity);
    ++n_reads_;
    return data_[key];
  }
};

const uint32_t kMainStorageSize = 1000000;
const uint32_t kSizeM = (kMainStorageSize / 100) * 9;
const uint32_t kSizeS = (kMainStorageSize / 100) * 1;

MainStorage<kMainStorageSize> gMainStorage;

int main() {
  S3FIFO<uint32_t, uint64_t, kSizeS, kSizeM, MainStorage<kMainStorageSize>>
      cache(gMainStorage);
  uint32_t object = 0;
  uint64_t ops = 0;
  while (std::cin >> object) {
    cache.Read(object);
    ++ops;
  }
  double hit_ratio = 1.0 - double(gMainStorage.n_reads_) / ops;
  std::cout << "Hit ratio: " << hit_ratio << '\n';
}