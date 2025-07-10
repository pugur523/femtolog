// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGING_IMPL_SPMC_QUEUE_H_
#define INCLUDE_FEMTOLOG_LOGGING_IMPL_SPMC_QUEUE_H_

#include <atomic>
#include <cstddef>
#include <memory>

#include "femtolog/core/base/memory_util.h"
#include "femtolog/core/check.h"
#include "femtolog/logging/base/logging_export.h"

namespace femtolog::logging {

enum class SpmcQueueStatus : uint8_t {
  kOk = 0,
  kUninitialized = 1,
  kUnderflow = 2,
  kOverflow = 3,
  kSizeIsZero = 4,
};

class FEMTOLOG_LOGGING_EXPORT SpmcQueue {
 public:
  SpmcQueue();
  ~SpmcQueue() = default;

  SpmcQueue(const SpmcQueue&) = delete;
  SpmcQueue& operator=(const SpmcQueue&) = delete;

  SpmcQueue(SpmcQueue&&) noexcept = delete;
  SpmcQueue& operator=(SpmcQueue&&) noexcept = delete;

  void reserve(std::size_t capacity_bytes);

  template <typename T>
  inline SpmcQueueStatus enqueue_bytes(const T* data) noexcept {
    return enqueue_bytes(reinterpret_cast<const void*>(data), sizeof(T));
  }
  SpmcQueueStatus enqueue_bytes(const void* data_ptr,
                                std::size_t data_size) noexcept;

  template <typename T>
  inline SpmcQueueStatus dequeue_bytes(T* data) noexcept {
    return dequeue_bytes(reinterpret_cast<void*>(data), sizeof(T));
  }
  SpmcQueueStatus dequeue_bytes(void* data_ptr, std::size_t data_size) noexcept;

  template <typename T>
  inline SpmcQueueStatus peek_bytes(T* data) const noexcept {
    return peek_bytes(reinterpret_cast<void*>(data), sizeof(T));
  }
  SpmcQueueStatus peek_bytes(void* data_ptr,
                             std::size_t data_size) const noexcept;

  [[nodiscard]] inline bool empty() const noexcept {
    const std::size_t head = head_idx_.load(std::memory_order_acquire);
    const std::size_t tail = tail_idx_.load(std::memory_order_acquire);
    return head == tail;
  }

  [[nodiscard]] inline std::size_t size() const noexcept {
    FEMTOLOG_DCHECK(buffer_);
    const std::size_t head = head_idx_.load(std::memory_order_acquire);
    const std::size_t tail = tail_idx_.load(std::memory_order_acquire);
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
  SpmcQueueStatus enqueue_bulk(const void* const* data_ptrs,
                               const std::size_t* data_sizes,
                               std::size_t count) noexcept;

  SpmcQueueStatus dequeue_bulk(void* const* data_ptrs,
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

  // Consumer side (read-mostly, separate cache line)
  // Using ticket-based approach for multiple consumers
  alignas(std::hardware_destructive_interference_size)
      std::atomic<std::size_t> head_idx_ = 0;
  alignas(std::hardware_destructive_interference_size)
      std::atomic<std::size_t> commit_idx_ = 0;

  // Buffer management
  alignas(std::hardware_destructive_interference_size)
      std::unique_ptr<std::byte[], core::AlignedDeleter> buffer_deleter_ =
          nullptr;
  std::size_t allocation_size_ = 0;
};

// static
constexpr std::size_t SpmcQueue::next_power_of_2(std::size_t n) noexcept {
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

#endif  // INCLUDE_FEMTOLOG_LOGGING_IMPL_SPMC_QUEUE_H_
