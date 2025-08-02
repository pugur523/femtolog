// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/spsc_queue.h"

#include <cstring>
#include <memory>
#include <new>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/memory_util.h"
#include "femtolog/core/check.h"

namespace femtolog::logging {

SpscQueue::SpscQueue() : buffer_(nullptr), buffer_deleter_(nullptr) {}

void SpscQueue::reserve(std::size_t capacity_bytes) {
  FEMTOLOG_DCHECK_GT(capacity_bytes, 0);

  // Ensure capacity is power of 2 for efficient bitwise operations
  const std::size_t capacity = next_power_of_2(capacity_bytes);

  // Allocate cache-line aligned buffer for optimal memory access
  // Use larger alignment for better performance on modern CPUs
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
    head_cached_ = 0;
    tail_cached_ = 0;
    head_cached_snapshot_ = 0;
    tail_cached_snapshot_ = 0;
    return;
  }

  buffer_ = new_buffer;
  buffer_deleter_.reset(new_buffer);
  capacity_ = capacity;
  mask_ = capacity - 1;
  allocation_size_ = alloc_size;

  head_idx_.store(0, std::memory_order_relaxed);
  tail_idx_.store(0, std::memory_order_relaxed);
  head_cached_ = 0;
  tail_cached_ = 0;
  head_cached_snapshot_ = 0;
  tail_cached_snapshot_ = 0;
}

SpscQueueStatus SpscQueue::enqueue_bytes(const void* data_ptr,
                                         std::size_t data_size) noexcept {
  if (!buffer_) [[unlikely]] {
    return SpscQueueStatus::kUninitialized;
  }
  if (data_size == 0) [[unlikely]] {
    return SpscQueueStatus::kSizeIsZero;
  }

  const std::size_t current_tail = tail_idx_.load(std::memory_order_relaxed);

  // Use cached snapshot to reduce atomic loads
  std::size_t current_head = head_cached_snapshot_;
  const std::size_t used_space = current_tail - current_head;

  if (capacity_ - used_space < data_size) [[unlikely]] {
    current_head = head_idx_.load(std::memory_order_acquire);
    head_cached_snapshot_ = current_head;

    const std::size_t new_used_space = current_tail - current_head;
    if (capacity_ - new_used_space < data_size) [[unlikely]] {
      return SpscQueueStatus::kOverflow;
    }
  }

  const std::size_t tail_pos = current_tail & mask_;
  const std::size_t space_to_end = (mask_ + 1) - tail_pos;

  if (data_size <= space_to_end) [[likely]] {
    std::memcpy(buffer_ + tail_pos, data_ptr, data_size);
  } else {
    // wrap-around
    std::memcpy(buffer_ + tail_pos, data_ptr, space_to_end);
    std::memcpy(buffer_, static_cast<const char*>(data_ptr) + space_to_end,
                data_size - space_to_end);
  }

  std::atomic_thread_fence(std::memory_order_release);
  tail_idx_.store(current_tail + data_size, std::memory_order_relaxed);
  tail_cached_ = current_tail + data_size;
  return SpscQueueStatus::kOk;
}

SpscQueueStatus SpscQueue::dequeue_bytes(void* data_ptr,
                                         std::size_t data_size) noexcept {
  if (!buffer_) [[unlikely]] {
    return SpscQueueStatus::kUninitialized;
  }
  if (data_size == 0) [[unlikely]] {
    return SpscQueueStatus::kSizeIsZero;
  }

  const std::size_t current_head = head_idx_.load(std::memory_order_relaxed);

  // Use cached snapshot to reduce atomic loads
  std::size_t current_tail = tail_cached_snapshot_;
  const std::size_t available_data = current_tail - current_head;

  if (available_data < data_size) [[unlikely]] {
    current_tail = tail_idx_.load(std::memory_order_acquire);
    tail_cached_snapshot_ = current_tail;

    const std::size_t new_available = current_tail - current_head;
    if (new_available < data_size) [[unlikely]] {
      return SpscQueueStatus::kUnderflow;
    }
  }

  const std::size_t head_pos = current_head & mask_;
  const std::size_t bytes_to_end = (mask_ + 1) - head_pos;

  if (data_size <= bytes_to_end) [[likely]] {
    std::memcpy(data_ptr, buffer_ + head_pos, data_size);
  } else {
    // wrap-around
    std::memcpy(data_ptr, buffer_ + head_pos, bytes_to_end);
    std::memcpy(static_cast<char*>(data_ptr) + bytes_to_end, buffer_,
                data_size - bytes_to_end);
  }

  std::atomic_thread_fence(std::memory_order_release);
  head_idx_.store(current_head + data_size, std::memory_order_relaxed);
  head_cached_ = current_head + data_size;
  return SpscQueueStatus::kOk;
}

SpscQueueStatus SpscQueue::peek_bytes(void* data_ptr,
                                      std::size_t data_size) const noexcept {
  if (!buffer_) [[unlikely]] {
    return SpscQueueStatus::kUninitialized;
  }
  if (data_size == 0) [[unlikely]] {
    return SpscQueueStatus::kSizeIsZero;
  }

  const std::size_t current_head = head_idx_.load(std::memory_order_relaxed);
  const std::size_t current_tail = tail_idx_.load(std::memory_order_relaxed);

  const std::size_t available_data = current_tail - current_head;
  if (data_size > available_data) [[unlikely]] {
    return SpscQueueStatus::kUnderflow;
  }

  const std::size_t head_pos = current_head & mask_;
  const std::size_t bytes_to_end = (mask_ + 1) - head_pos;

  if (data_size <= bytes_to_end) [[likely]] {
    std::memcpy(data_ptr, buffer_ + head_pos, data_size);
  } else {
    // wrap-around
    std::memcpy(data_ptr, buffer_ + head_pos, bytes_to_end);
    std::memcpy(static_cast<char*>(data_ptr) + bytes_to_end, buffer_,
                data_size - bytes_to_end);
  }

  return SpscQueueStatus::kOk;
}

SpscQueueStatus SpscQueue::enqueue_bulk(const void* const* data_ptrs,
                                        const std::size_t* data_sizes,
                                        std::size_t count) noexcept {
  if (!buffer_) [[unlikely]] {
    return SpscQueueStatus::kUninitialized;
  }
  if (count == 0) [[unlikely]] {
    return SpscQueueStatus::kSizeIsZero;
  }

  // Calculate total size needed
  std::size_t total_size = 0;
  for (std::size_t i = 0; i < count; ++i) {
    total_size += data_sizes[i];
  }

  if (total_size == 0) [[unlikely]] {
    return SpscQueueStatus::kSizeIsZero;
  }

  const std::size_t current_tail = tail_idx_.load(std::memory_order_relaxed);
  std::size_t current_head = head_cached_snapshot_;
  const std::size_t used_space = current_tail - current_head;

  if (capacity_ - used_space < total_size) [[unlikely]] {
    current_head = head_idx_.load(std::memory_order_acquire);
    head_cached_snapshot_ = current_head;

    const std::size_t new_used_space = current_tail - current_head;
    if (capacity_ - new_used_space < total_size) [[unlikely]] {
      return SpscQueueStatus::kOverflow;
    }
  }

  // Copy all data
  std::size_t offset = 0;
  for (std::size_t i = 0; i < count; ++i) {
    const std::size_t data_size = data_sizes[i];
    const std::size_t tail_pos = (current_tail + offset) & mask_;
    const std::size_t space_to_end = (mask_ + 1) - tail_pos;

    if (data_size <= space_to_end) [[likely]] {
      std::memcpy(buffer_ + tail_pos, data_ptrs[i], data_size);
    } else {
      std::memcpy(buffer_ + tail_pos, data_ptrs[i], space_to_end);
      std::memcpy(buffer_,
                  static_cast<const char*>(data_ptrs[i]) + space_to_end,
                  data_size - space_to_end);
    }
    offset += data_size;
  }

  std::atomic_thread_fence(std::memory_order_release);
  tail_idx_.store(current_tail + total_size, std::memory_order_relaxed);
  tail_cached_ = current_tail + total_size;
  return SpscQueueStatus::kOk;
}

SpscQueueStatus SpscQueue::dequeue_bulk(void* const* data_ptrs,
                                        const std::size_t* data_sizes,
                                        std::size_t count) noexcept {
  if (!buffer_) [[unlikely]] {
    return SpscQueueStatus::kUninitialized;
  }
  if (count == 0) [[unlikely]] {
    return SpscQueueStatus::kSizeIsZero;
  }

  // Calculate total size needed
  std::size_t total_size = 0;
  for (std::size_t i = 0; i < count; ++i) {
    total_size += data_sizes[i];
  }

  if (total_size == 0) [[unlikely]] {
    return SpscQueueStatus::kSizeIsZero;
  }

  const std::size_t current_head = head_idx_.load(std::memory_order_relaxed);
  std::size_t current_tail = tail_cached_snapshot_;
  const std::size_t available_data = current_tail - current_head;

  if (available_data < total_size) [[unlikely]] {
    current_tail = tail_idx_.load(std::memory_order_acquire);
    tail_cached_snapshot_ = current_tail;

    const std::size_t new_available = current_tail - current_head;
    if (new_available < total_size) [[unlikely]] {
      return SpscQueueStatus::kUnderflow;
    }
  }

  // Copy all data
  std::size_t offset = 0;
  for (std::size_t i = 0; i < count; ++i) {
    const std::size_t data_size = data_sizes[i];
    const std::size_t head_pos = (current_head + offset) & mask_;
    const std::size_t bytes_to_end = (mask_ + 1) - head_pos;

    if (data_size <= bytes_to_end) [[likely]] {
      std::memcpy(data_ptrs[i], buffer_ + head_pos, data_size);
    } else {
      std::memcpy(data_ptrs[i], buffer_ + head_pos, bytes_to_end);
      std::memcpy(static_cast<char*>(data_ptrs[i]) + bytes_to_end, buffer_,
                  data_size - bytes_to_end);
    }
    offset += data_size;
  }

  std::atomic_thread_fence(std::memory_order_release);
  head_idx_.store(current_head + total_size, std::memory_order_relaxed);
  head_cached_ = current_head + total_size;
  return SpscQueueStatus::kOk;
}

}  // namespace femtolog::logging
