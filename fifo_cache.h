#pragma once

#include <cassert>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>

#include "ring_buffer.h"

template <typename K, uint32_t Capacity> class Fifo {
public:
  Fifo() {
    stored_items_.reserve(Capacity);
    for (uint32_t i = 0; i < Capacity; i++) {
      unused_loc_.Push(i);
    }
  }

  bool Contains(const K &key) const {
    return stored_items_.find(key) != stored_items_.end();
  }

  uint32_t GetLocation(const K &key) const { return stored_items_.at(key); }
  uint32_t Size() const { return stored_items_.size(); }
  bool IsFull() const { return Size() == Capacity; }

  uint32_t Evict(K &evicted_key_out, bool keep_loc = false) {
    assert(Size() > 0);
    evicted_key_out = eviction_queue_.Pop();
    assert(stored_items_.find(evicted_key_out) != stored_items_.end());
    uint32_t result = stored_items_[evicted_key_out];
    stored_items_.erase(evicted_key_out);
    if (!keep_loc) {
      unused_loc_.Push(result);
    }
    return result;
  }

  void MarkFreeLoc(uint32_t loc) { unused_loc_.Push(loc); }

  uint32_t Insert(const K &key, uint32_t hint = Capacity) {
    assert(Size() < Capacity);
    assert(stored_items_.find(key) == stored_items_.end());
    uint32_t loc = hint;
    if (hint == Capacity) {
      loc = unused_loc_.Pop();
    }
    stored_items_[key] = loc;
    eviction_queue_.Push(key);
    return loc;
  }

private:
  std::unordered_map<K, uint32_t> stored_items_;
  RingBuffer<uint32_t, Capacity> unused_loc_;
  RingBuffer<K, Capacity> eviction_queue_;
};