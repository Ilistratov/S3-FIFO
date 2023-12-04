#pragma once

#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

#include "ring_buffer.h"

template <typename K, typename V, int SizeS, int SizeM,
          typename UnderlyingStorage>
class S3FIFO {
public:
  S3FIFO(UnderlyingStorage &storage) : underlying_storage_(storage) {
    for (uint32_t i = 0; i < SizeS + SizeM; i++) {
      free_loc_.Push(i);
    }
  }
  void Write(const K &key, V value) { Get(key) = value; }
  V Read(const K &key) { return Get(key); }

private:
  V &Get(const K &key) {
    auto it = key_to_loc_.find(key);
    uint32_t loc = 0;
    if (it == key_to_loc_.end()) {
      Insert(key, underlying_storage_.Read(key));
      loc = key_to_loc_.at(key);
      assert(loc < SizeS + SizeM);
    } else {
      loc = it->second;
    }
    if (freq_[loc] < 3) {
      ++freq_[loc];
    }
    return data_[loc];
  }

  void Insert(const K &key, V value) {
    uint32_t loc = SizeS + SizeM;
    if (ghost_contents_.find(key) != ghost_contents_.end()) {
      loc = InsertToMain(key);
    } else {
      loc = InsertToSmall(key);
    }
    freq_[loc] = 0;
    key_to_loc_[key] = loc;
    data_[loc] = value;
  }

  uint32_t InsertToSmall(const K &key) {
    while (small_.IsFull()) {
      EvictSmall();
    }
    uint32_t loc = free_loc_.Pop();
    small_.Push(key);
    return loc;
  }

  uint32_t InsertToMain(const K &key) {
    while (main_.IsFull()) {
      EvictMain();
    }
    uint32_t loc = free_loc_.Pop();
    main_.Push(key);
    return loc;
  }

  void InsertToGhost(K key) {
    if (ghost_.IsFull()) {
      ghost_.Pop();
    }
    ghost_.Push(key);
  }

  void EvictToUnderlyingStorage(K key, uint32_t loc) {
    underlying_storage_.Write(key, data_[loc]);
    free_loc_.Push(loc);
    key_to_loc_.erase(key);
  }

  void EvictSmall() {
    K eviction_candidate = small_.Pop();
    uint32_t loc = key_to_loc_.at(eviction_candidate);
    /* In Algo 1. <=1
     * In Fig. 5 == 0
     *
     * With <= 1 hit-ratio 0.82
     * With == 0 hit-ratio 0.78
     */
    if (freq_[loc] <= 1) {
      EvictToUnderlyingStorage(eviction_candidate, loc);
      InsertToGhost(eviction_candidate);
      return;
    }
    if (main_.IsFull()) {
      EvictMain();
    }
    /* are we supposed to decrement freq??? */
    main_.Push(eviction_candidate);
  }

  void EvictMain() {
    while (main_.Size() > 0) {
      K eviction_candidate = main_.Pop();
      uint32_t loc = key_to_loc_.at(eviction_candidate);
      if (freq_[loc] == 0) {
        EvictToUnderlyingStorage(eviction_candidate, loc);
        break;
      }
      freq_[loc] -= 1;
      main_.Push(eviction_candidate);
    }
  }

private:
  RingBuffer<K, SizeS> small_;
  RingBuffer<K, SizeM> main_;
  RingBuffer<K, SizeM> ghost_;
  RingBuffer<uint32_t, SizeS + SizeM> free_loc_; /* could be reduced in size */
  std::unordered_map<K, uint32_t> key_to_loc_;
  std::unordered_set<K> ghost_contents_;
  V data_[SizeS + SizeM];
  uint8_t freq_[SizeS + SizeM]; /* could be reduced in size */
  UnderlyingStorage &underlying_storage_;
};
