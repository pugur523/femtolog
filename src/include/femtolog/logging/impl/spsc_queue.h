// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGING_IMPL_SPSC_QUEUE_H_
#define INCLUDE_FEMTOLOG_LOGGING_IMPL_SPSC_QUEUE_H_

#include <atomic>
#include <cstddef>
#include <memory>

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
  SpscQueueStatus dequeue_bytes(void* data_ptr, std::size_t size) noexcept;

  template <typename T>
  inline SpscQueueStatus peek_bytes(T* data) const noexcept {
    return peek_bytes(reinterpret_cast<void*>(data), sizeof(T));
  }
  SpscQueueStatus peek_bytes(void* data_ptr, std::size_t size) const noexcept;

  [[nodiscard]] inline bool empty() const noexcept {
    const std::size_t head = head_cached_;
    const std::size_t tail = tail_idx_.load(std::memory_order_relaxed);
    return head == tail;
  }

  [[nodiscard]] inline std::size_t size() const noexcept {
    DCHECK(buffer_);
    const std::size_t head = head_cached_;
    const std::size_t tail = tail_idx_.load(std::memory_order_relaxed);
    return tail - head;
  }

  [[nodiscard]] inline std::size_t capacity() const noexcept {
    return capacity_;
  }

  [[nodiscard]] inline std::size_t available_space() const noexcept {
    DCHECK(buffer_);
    return capacity_ - size();
  }

 private:
  [[nodiscard]] static constexpr std::size_t next_power_of_2(
      std::size_t n) noexcept;

  alignas(128) std::byte* buffer_;
  std::size_t capacity_ = 0;
  std::size_t mask_ = 0;

  // Producer side (write-mostly, separate cache line)
  alignas(128) std::atomic<std::size_t> tail_idx_ = 0;
  mutable std::size_t tail_cached_ = 0;

  // Consumer side (read-mostly, separate cache line)
  alignas(128) std::atomic<std::size_t> head_idx_ = 0;
  mutable std::size_t head_cached_ = 0;

  // Buffer management
  std::unique_ptr<std::byte[], void (*)(void*)> buffer_deleter_;
};

// static
constexpr std::size_t SpscQueue::next_power_of_2(std::size_t n) noexcept {
  if (n <= 1) {
    return 2;
  }
  if ((n & (n - 1)) == 0) {
    return n;
  }

  std::size_t result = 1;
  while (result < n) {
    result <<= 1;
  }
  return result;
}

}  // namespace femtolog::logging

#endif  // INCLUDE_FEMTOLOG_LOGGING_IMPL_SPSC_QUEUE_H_
