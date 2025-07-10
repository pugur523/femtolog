// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/spmc_queue.h"

#include <cstring>
#include <memory>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/memory_util.h"
#include "femtolog/core/check.h"

#if FEMTOLOG_ENABLE_AVX2
#include <immintrin.h>
#endif

namespace femtolog::logging {

SpmcQueue::SpmcQueue() : buffer_(nullptr), buffer_deleter_(nullptr) {}

void SpmcQueue::reserve(std::size_t capacity_bytes) {
  FEMTOLOG_DCHECK_GT(capacity_bytes, 0);

  const std::size_t capacity = next_power_of_2(capacity_bytes);
  constexpr std::size_t alignment = std::hardware_destructive_interference_size;
  const std::size_t alloc_size = capacity + alignment;

  std::byte* new_buffer = static_cast<std::byte*>(
      core::aligned_alloc_wrapper(alignment, alloc_size));

  if (!new_buffer) [[unlikely]] {
    buffer_deleter_.reset();
    buffer_ = nullptr;
    capacity_ = 0;
    mask_ = 0;
    allocation_size_ = 0;
    head_idx_.store(0, std::memory_order_relaxed);
    tail_idx_.store(0, std::memory_order_relaxed);
    commit_idx_.store(0, std::memory_order_relaxed);
    return;
  }

  buffer_ = new_buffer;
  buffer_deleter_.reset(new_buffer);
  capacity_ = capacity;
  mask_ = capacity - 1;
  allocation_size_ = alloc_size;

  head_idx_.store(0, std::memory_order_relaxed);
  tail_idx_.store(0, std::memory_order_relaxed);
  commit_idx_.store(0, std::memory_order_relaxed);
}

SpmcQueueStatus SpmcQueue::enqueue_bytes(const void* data_ptr,
                                         std::size_t data_size) noexcept {
  if (!buffer_) [[unlikely]] {
    return SpmcQueueStatus::kUninitialized;
  }
  if (data_size == 0) [[unlikely]] {
    return SpmcQueueStatus::kSizeIsZero;
  }

  // Check available space using cached value first
  const std::size_t current_tail = tail_idx_.load(std::memory_order_relaxed);
  const std::size_t current_commit =
      commit_idx_.load(std::memory_order_acquire);
  const std::size_t used_space = current_tail - current_commit;

  if (capacity_ - used_space < data_size) [[unlikely]] {
    return SpmcQueueStatus::kOverflow;
  }

  // Copy data to buffer (wrap around handling)
  const std::size_t tail_pos = current_tail & mask_;
  const std::size_t space_to_end = (mask_ + 1) - tail_pos;

  if (data_size <= space_to_end) {
    std::memcpy(buffer_ + tail_pos, data_ptr, data_size);
  } else {
    std::memcpy(buffer_ + tail_pos, data_ptr, space_to_end);
    std::memcpy(buffer_, static_cast<const char*>(data_ptr) + space_to_end,
                data_size - space_to_end);
  }

  // Memory barrier before updating tail
  std::atomic_thread_fence(std::memory_order_release);
  tail_idx_.store(current_tail + data_size, std::memory_order_relaxed);

  return SpmcQueueStatus::kOk;
}

SpmcQueueStatus SpmcQueue::dequeue_bytes(void* data_ptr,
                                         std::size_t data_size) noexcept {
  if (!buffer_) [[unlikely]] {
    return SpmcQueueStatus::kUninitialized;
  }
  if (data_size == 0) [[unlikely]] {
    return SpmcQueueStatus::kSizeIsZero;
  }

  // Ticket-based approach for multiple consumers
  const std::size_t ticket =
      head_idx_.fetch_add(data_size, std::memory_order_acq_rel);

  // Check if we have enough data available
  const std::size_t tail = tail_idx_.load(std::memory_order_acquire);
  if (tail - ticket < data_size) [[unlikely]] {
    // Rewind head and return underflow
    head_idx_.fetch_sub(data_size, std::memory_order_release);
    return SpmcQueueStatus::kUnderflow;
  }

  // Copy data from buffer (wrap around handling)
  const std::size_t head_pos = ticket & mask_;
  const std::size_t bytes_to_end = (mask_ + 1) - head_pos;

  if (data_size <= bytes_to_end) {
    std::memcpy(data_ptr, buffer_ + head_pos, data_size);
  } else {
    std::memcpy(data_ptr, buffer_ + head_pos, bytes_to_end);
    std::memcpy(static_cast<char*>(data_ptr) + bytes_to_end, buffer_,
                data_size - bytes_to_end);
  }

  // Wait for all previous consumers to complete and update commit index
  std::size_t expected_commit = ticket;
  while (!commit_idx_.compare_exchange_weak(expected_commit, ticket + data_size,
                                            std::memory_order_release,
                                            std::memory_order_relaxed)) {
    expected_commit = ticket;
#if FEMTOLOG_ENABLE_AVX2
    _mm_pause();
#endif
  }

  return SpmcQueueStatus::kOk;
}

SpmcQueueStatus SpmcQueue::peek_bytes(void* data_ptr,
                                      std::size_t data_size) const noexcept {
  if (!buffer_) [[unlikely]] {
    return SpmcQueueStatus::kUninitialized;
  }
  if (data_size == 0) [[unlikely]] {
    return SpmcQueueStatus::kSizeIsZero;
  }

  const std::size_t current_commit =
      commit_idx_.load(std::memory_order_acquire);
  const std::size_t current_tail = tail_idx_.load(std::memory_order_acquire);

  if (current_tail - current_commit < data_size) [[unlikely]] {
    return SpmcQueueStatus::kUnderflow;
  }

  // Copy data from buffer (wrap around handling)
  const std::size_t head_pos = current_commit & mask_;
  const std::size_t bytes_to_end = (mask_ + 1) - head_pos;

  if (data_size <= bytes_to_end) {
    std::memcpy(data_ptr, buffer_ + head_pos, data_size);
  } else {
    std::memcpy(data_ptr, buffer_ + head_pos, bytes_to_end);
    std::memcpy(static_cast<char*>(data_ptr) + bytes_to_end, buffer_,
                data_size - bytes_to_end);
  }

  return SpmcQueueStatus::kOk;
}

SpmcQueueStatus SpmcQueue::enqueue_bulk(const void* const* data_ptrs,
                                        const std::size_t* data_sizes,
                                        std::size_t count) noexcept {
  if (!buffer_) [[unlikely]] {
    return SpmcQueueStatus::kUninitialized;
  }
  if (count == 0) [[unlikely]] {
    return SpmcQueueStatus::kSizeIsZero;
  }

  // Calculate total size
  std::size_t total_size = 0;
  for (std::size_t i = 0; i < count; ++i) {
    total_size += data_sizes[i];
  }

  if (total_size == 0) [[unlikely]] {
    return SpmcQueueStatus::kSizeIsZero;
  }

  // Check available space
  const std::size_t current_tail = tail_idx_.load(std::memory_order_relaxed);
  const std::size_t current_commit =
      commit_idx_.load(std::memory_order_acquire);
  const std::size_t used_space = current_tail - current_commit;

  if (capacity_ - used_space < total_size) [[unlikely]] {
    return SpmcQueueStatus::kOverflow;
  }

  // Copy all data
  std::size_t offset = 0;
  for (std::size_t i = 0; i < count; ++i) {
    const std::size_t data_size = data_sizes[i];
    const std::size_t tail_pos = (current_tail + offset) & mask_;
    const std::size_t space_to_end = (mask_ + 1) - tail_pos;

    if (data_size <= space_to_end) {
      std::memcpy(buffer_ + tail_pos, data_ptrs[i], data_size);
    } else {
      std::memcpy(buffer_ + tail_pos, data_ptrs[i], space_to_end);
      std::memcpy(buffer_,
                  static_cast<const char*>(data_ptrs[i]) + space_to_end,
                  data_size - space_to_end);
    }
    offset += data_size;
  }

  // Memory barrier before updating tail
  std::atomic_thread_fence(std::memory_order_release);
  tail_idx_.store(current_tail + total_size, std::memory_order_relaxed);

  return SpmcQueueStatus::kOk;
}

SpmcQueueStatus SpmcQueue::dequeue_bulk(void* const* data_ptrs,
                                        const std::size_t* data_sizes,
                                        std::size_t count) noexcept {
  if (!buffer_) [[unlikely]] {
    return SpmcQueueStatus::kUninitialized;
  }
  if (count == 0) [[unlikely]] {
    return SpmcQueueStatus::kSizeIsZero;
  }

  // Calculate total size
  std::size_t total_size = 0;
  for (std::size_t i = 0; i < count; ++i) {
    total_size += data_sizes[i];
  }

  if (total_size == 0) [[unlikely]] {
    return SpmcQueueStatus::kSizeIsZero;
  }

  // Ticket-based approach
  const std::size_t ticket =
      head_idx_.fetch_add(total_size, std::memory_order_acq_rel);

  // Check if we have enough data
  const std::size_t tail = tail_idx_.load(std::memory_order_acquire);
  if (tail - ticket < total_size) [[unlikely]] {
    // Rewind head and return underflow
    head_idx_.fetch_sub(total_size, std::memory_order_release);
    return SpmcQueueStatus::kUnderflow;
  }

  // Copy all data
  std::size_t offset = 0;
  for (std::size_t i = 0; i < count; ++i) {
    const std::size_t data_size = data_sizes[i];
    const std::size_t head_pos = (ticket + offset) & mask_;
    const std::size_t bytes_to_end = (mask_ + 1) - head_pos;

    if (data_size <= bytes_to_end) {
      std::memcpy(data_ptrs[i], buffer_ + head_pos, data_size);
    } else {
      std::memcpy(data_ptrs[i], buffer_ + head_pos, bytes_to_end);
      std::memcpy(static_cast<char*>(data_ptrs[i]) + bytes_to_end, buffer_,
                  data_size - bytes_to_end);
    }
    offset += data_size;
  }

  // Wait for all previous consumers to complete and update commit index
  std::size_t expected_commit = ticket;
  while (!commit_idx_.compare_exchange_weak(
      expected_commit, ticket + total_size, std::memory_order_release,
      std::memory_order_relaxed)) {
    expected_commit = ticket;
#if FEMTOLOG_ENABLE_AVX2
    _mm_pause();
#endif
  }

  return SpmcQueueStatus::kOk;
}

}  // namespace femtolog::logging
