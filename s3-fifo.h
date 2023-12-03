#pragma once

#include "fifo_cache.h"

template <typename K, typename V, int SizeS, int SizeM,
          typename UnderlyingStorage>
class S3FIFO {
public:
  S3FIFO(UnderlyingStorage &storage) : underlying_storage_(storage) {}
  void Write(const K &key, V value) { Get(key) = value; }
  V Read(const K &key) { return Get(key); }

private:
  uint32_t GetLoc(const K &key) const {
    uint32_t loc = SizeS + SizeM;
    if (fifo_small_.Contains(key)) {
      loc = fifo_small_.GetLocation(key);
    } else if (fifo_main_.Contains(key)) {
      loc = SizeS + fifo_main_.GetLocation(key);
    }
    return loc;
  }

  V &Get(const K &key) {
    uint32_t loc = GetLoc(key);
    if (loc >= SizeS + SizeM) {
      Insert(key, underlying_storage_.Read(key));
      loc = GetLoc(key);
      assert(loc < SizeS + SizeM);
    }
    if (loc < SizeS + SizeM) {
      if (freq_[loc] < 3) {
        ++freq_[loc];
      }
      return data_[loc];
    }
    Insert(key, underlying_storage_.Read(key));
    return Get(key);
  }

  void Insert(const K &key, V value) {
    while (fifo_small_.IsFull() && fifo_main_.IsFull()) {
      Evict();
    }
    if (fifo_ghost_.Contains(key)) {
      fifo_main_.Insert(key);
    } else {
      fifo_small_.Insert(key);
    }
    uint32_t loc = GetLoc(key);
    freq_[loc] = 0;
    data_[loc] = value;
  }

  void Evict() {
    if (fifo_small_.IsFull()) {
      EvictS();
    } else {
      EvictM();
    }
  }

  void EvictS() {
    while (fifo_small_.Size() > 0) {
      K eviction_candidate;
      uint32_t loc = fifo_small_.Evict(eviction_candidate);
      if (freq_[loc] <= 1) {
        if (fifo_ghost_.IsFull()) {
          K unused;
          fifo_ghost_.Evict(unused);
        }
        fifo_ghost_.Insert(eviction_candidate);
        underlying_storage_.Write(eviction_candidate, data_[loc]);
        break;
      }
      if (fifo_main_.IsFull()) {
        EvictM();
      }
    }
  }

  void EvictM() {
    while (fifo_main_.Size() > 0) {
      K eviction_candidate;
      uint32_t loc = fifo_main_.Evict(eviction_candidate, true);
      if (freq_[loc + SizeS] > 0) {
        fifo_main_.Insert(eviction_candidate, loc);
        freq_[loc + SizeS] -= 1;
      } else {
        underlying_storage_.Write(eviction_candidate, data_[loc + SizeS]);
        fifo_small_.MarkFreeLoc(loc);
        break;
      }
    }
  }

private:
  Fifo<K, SizeS> fifo_small_;
  Fifo<K, SizeM> fifo_main_;
  Fifo<K, SizeM> fifo_ghost_;
  uint8_t freq_[SizeS + SizeM];
  V data_[SizeS + SizeM];
  UnderlyingStorage &underlying_storage_;
};