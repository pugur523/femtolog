// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGING_IMPL_SPSC_QUEUE_H_
#define INCLUDE_FEMTOLOG_LOGGING_IMPL_SPSC_QUEUE_H_

#include <atomic>
#include <cstddef>
#include <memory>
#include <new>

#include "femtolog/core/base/memory_util.h"
#include "femtolog/core/check.h"
#include "femtolog/logging/base/logging_export.h"

namespace femtolog::logging {

enum class SpscQueueStatus : uint8_t {
  kOk = 0,
  kUninitialized = 1,
  kUnderflow = 2,
  kOverflow = 3,
  kSizeIsZero = 4,
};

class FEMTOLOG_LOGGING_EXPORT SpscQueue {
 public:
  SpscQueue();
  ~SpscQueue() = default;

  SpscQueue(const SpscQueue&) = delete;
  SpscQueue& operator=(const SpscQueue&) = delete;

  SpscQueue(SpscQueue&&) noexcept = delete;
  SpscQueue& operator=(SpscQueue&&) noexcept = delete;

  void reserve(std::size_t capacity_bytes);

  template <typename T>
  inline SpscQueueStatus enqueue_bytes(const T* data) noexcept {
    return enqueue_bytes(reinterpret_cast<const void*>(data), sizeof(T));
  }
  SpscQueueStatus enqueue_bytes(const void* data_ptr,
                                std::size_t data_size) noexcept;

  template <typename T>
  inline SpscQueueStatus dequeue_bytes(T* data) noexcept {
    return dequeue_bytes(reinterpret_cast<void*>(data), sizeof(T));
  }
  SpscQueueStatus dequeue_bytes(void* data_ptr, std::size_t data_size) noexcept;

  template <typename T>
  inline SpscQueueStatus peek_bytes(T* data) const noexcept {
    return peek_bytes(reinterpret_cast<void*>(data), sizeof(T));
  }
  SpscQueueStatus peek_bytes(void* data_ptr,
                             std::size_t data_size) const noexcept;

  [[nodiscard]] inline bool empty() const noexcept {
    const std::size_t head = head_cached_;
    const std::size_t tail = tail_idx_.load(std::memory_order_relaxed);
    return head == tail;
  }

  [[nodiscard]] inline std::size_t size() const noexcept {
    FEMTOLOG_DCHECK(buffer_);
    const std::size_t head = head_cached_;
    const std::size_t tail = tail_idx_.load(std::memory_order_relaxed);
    return tail - head;
  }

  [[nodiscard]] inline std::size_t capacity() const noexcept {
    return capacity_;
  }

  [[nodiscard]] inline std::size_t available_space() const noexcept {
    FEMTOLOG_DCHECK(buffer_);
    return capacity_ - size();
  }

  // Bulk operations for better performance
  SpscQueueStatus enqueue_bulk(const void* const* data_ptrs,
                               const std::size_t* data_sizes,
                               std::size_t count) noexcept;

  SpscQueueStatus dequeue_bulk(void* const* data_ptrs,
                               const std::size_t* data_sizes,
                               std::size_t count) noexcept;

  static constexpr std::size_t kBatchSize = 32;

 private:
  [[nodiscard]] static constexpr std::size_t next_power_of_2(
      std::size_t n) noexcept;

  // Cache line aligned buffer
  alignas(std::hardware_destructive_interference_size) std::byte* buffer_;
  std::size_t capacity_ = 0;
  std::size_t mask_ = 0;

  // Producer side (write-mostly, separate cache line)
  alignas(std::hardware_destructive_interference_size)
      std::atomic<std::size_t> tail_idx_ = 0;
  mutable std::size_t tail_cached_ = 0;
  // Cached snapshot of head for producer
  mutable std::size_t head_cached_snapshot_ = 0;

  // Consumer side (read-mostly, separate cache line)
  alignas(std::hardware_destructive_interference_size)
      std::atomic<std::size_t> head_idx_ = 0;
  mutable std::size_t head_cached_ = 0;
  // Cached snapshot of tail for consumer
  mutable std::size_t tail_cached_snapshot_ = 0;

  // Buffer management
  alignas(std::hardware_destructive_interference_size)
      std::unique_ptr<std::byte[], core::AlignedDeleter> buffer_deleter_ =
          nullptr;
  std::size_t allocation_size_ = 0;
};

// static
constexpr std::size_t SpscQueue::next_power_of_2(std::size_t n) noexcept {
  if (n <= 1) {
    return 2;
  }
  if ((n & (n - 1)) == 0) {
    return n;
  }

  // Use builtin count loading zeros for better performance
  return std::size_t(1) << (64 - __builtin_clzll(n - 1));
}

}  // namespace femtolog::logging

#endif  // INCLUDE_FEMTOLOG_LOGGING_IMPL_SPSC_QUEUE_H_
