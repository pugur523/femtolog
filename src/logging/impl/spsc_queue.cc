// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/spsc_queue.h"

#include <cstring>
#include <memory>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/check.h"

#if FEMTOLOG_IS_WINDOWS
#include <windows.h>
#elif FEMTOLOG_IS_LINUX
#include <sys/mman.h>
#endif

#if FEMTOLOG_ENABLE_AVX2
#include <immintrin.h>
#endif

namespace femtolog::logging {

namespace {

// Optimized memory allocation with proper alignment
void* aligned_alloc_wrapper(std::size_t alignment, std::size_t size) {
#if FEMTOLOG_IS_WINDOWS
  return _aligned_malloc(size, alignment);
#else
  void* ptr = nullptr;
  if (posix_memalign(&ptr, alignment, size) != 0) {
    return nullptr;
  }
  return ptr;
#endif
}

void aligned_free_wrapper(void* ptr) {
#if FEMTOLOG_IS_WINDOWS
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}

void optimized_copy(void* dest, const void* src, std::size_t size) noexcept {
#if FEMTOLOG_ENABLE_AVX2
  if (size >= SpscQueue::kPrefetchThreshold) {
    // prefetch if the size is greater than the threshold
    const char* src_ptr = static_cast<const char*>(src);
    char* dest_ptr = static_cast<char*>(dest);

    _mm_prefetch(src_ptr, _MM_HINT_T0);
    _mm_prefetch(dest_ptr, _MM_HINT_T0);

    // prefetch per 64bytes unit
    for (std::size_t i = 64; i < size; i += 64) {
      _mm_prefetch(src_ptr + i, _MM_HINT_T0);
      _mm_prefetch(dest_ptr + i, _MM_HINT_T0);
    }
  }
#endif

  std::memcpy(dest, src, size);
}

}  // namespace

SpscQueue::SpscQueue()
    : buffer_(nullptr), buffer_deleter_(nullptr, [](void*) {}) {}

void SpscQueue::reserve(std::size_t capacity_bytes) {
  FEMTOLOG_DCHECK_GT(capacity_bytes, 0);

  // Ensure capacity is power of 2 for efficient bitwise operations
  const std::size_t capacity = next_power_of_2(capacity_bytes);

  // Allocate cache-line aligned buffer for optimal memory access
  std::byte* new_buffer =
      static_cast<std::byte*>(aligned_alloc_wrapper(128, capacity));

  if (!new_buffer) {
    buffer_deleter_.reset();
    buffer_ = nullptr;
    capacity_ = 0;
    mask_ = 0;

    head_idx_.store(0, std::memory_order_relaxed);
    tail_idx_.store(0, std::memory_order_relaxed);
    head_cached_ = 0;
    tail_cached_ = 0;
    return;
  }

  // buffer_deleter_.reset(new_buffer);
  buffer_deleter_ = std::unique_ptr<std::byte[], void (*)(void*)>(
      new_buffer, aligned_free_wrapper);

  buffer_ = new_buffer;
  capacity_ = capacity;
  mask_ = capacity - 1;
  head_idx_.store(0, std::memory_order_relaxed);
  tail_idx_.store(0, std::memory_order_relaxed);
  head_cached_ = 0;
  tail_cached_ = 0;
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

  std::size_t current_head = head_cached_;
  const std::size_t used_space = current_tail - current_head;

  if (capacity_ - used_space < data_size) [[unlikely]] {
    current_head = head_idx_.load(std::memory_order_acquire);
    head_cached_ = current_head;

    const std::size_t new_used_space = current_tail - current_head;
    if (capacity_ - new_used_space < data_size) [[unlikely]] {
      return SpscQueueStatus::kOverflow;
    }
  }

  const std::size_t tail_pos = current_tail & mask_;
  const std::size_t space_to_end = (mask_ + 1) - tail_pos;

  if (data_size <= space_to_end) [[likely]] {
    optimized_copy(buffer_ + tail_pos, data_ptr, data_size);
  } else {
    optimized_copy(buffer_ + tail_pos, data_ptr, space_to_end);
    optimized_copy(buffer_, static_cast<const char*>(data_ptr) + space_to_end,
                   data_size - space_to_end);
  }

  std::atomic_thread_fence(std::memory_order_release);
  tail_idx_.store(current_tail + data_size, std::memory_order_relaxed);
  tail_cached_ = current_tail + data_size;
  return SpscQueueStatus::kOk;
}

SpscQueueStatus SpscQueue::dequeue_bytes(void* data_ptr,
                                         std::size_t size) noexcept {
  if (!buffer_) [[unlikely]] {
    return SpscQueueStatus::kUninitialized;
  }
  if (size == 0) [[unlikely]] {
    return SpscQueueStatus::kSizeIsZero;
  }

  const std::size_t current_head = head_idx_.load(std::memory_order_relaxed);

  std::size_t current_tail = tail_cached_;
  const std::size_t available_data = current_tail - current_head;

  if (available_data < size) [[unlikely]] {
    current_tail = tail_idx_.load(std::memory_order_acquire);
    tail_cached_ = current_tail;

    const std::size_t new_available = current_tail - current_head;
    if (new_available < size) [[unlikely]] {
      return SpscQueueStatus::kUnderflow;
    }
  }

  const std::size_t head_pos = current_head & mask_;
  const std::size_t bytes_to_end = (mask_ + 1) - head_pos;

  if (size <= bytes_to_end) [[likely]] {
    optimized_copy(data_ptr, buffer_ + head_pos, size);
  } else {
    optimized_copy(data_ptr, buffer_ + head_pos, bytes_to_end);
    optimized_copy(static_cast<char*>(data_ptr) + bytes_to_end, buffer_,
                   size - bytes_to_end);
  }

  std::atomic_thread_fence(std::memory_order_release);
  head_idx_.store(current_head + size, std::memory_order_relaxed);
  head_cached_ = current_head + size;
  return SpscQueueStatus::kOk;
}

SpscQueueStatus SpscQueue::peek_bytes(void* data_ptr,
                                      std::size_t size) const noexcept {
  if (!buffer_) [[unlikely]] {
    return SpscQueueStatus::kUninitialized;
  }
  if (size == 0) [[unlikely]] {
    return SpscQueueStatus::kSizeIsZero;
  }

  const std::size_t current_head = head_idx_.load(std::memory_order_relaxed);
  const std::size_t current_tail = tail_idx_.load(std::memory_order_acquire);

  const std::size_t available_data = current_tail - current_head;
  if (size > available_data) [[unlikely]] {
    return SpscQueueStatus::kUnderflow;
  }

  const std::size_t head_pos = current_head & mask_;
  const std::size_t bytes_to_end = (mask_ + 1) - head_pos;

  if (size <= bytes_to_end) [[likely]] {
    optimized_copy(data_ptr, buffer_ + head_pos, size);
  } else {
    optimized_copy(data_ptr, buffer_ + head_pos, bytes_to_end);
    optimized_copy(static_cast<char*>(data_ptr) + bytes_to_end, buffer_,
                   size - bytes_to_end);
  }

  return SpscQueueStatus::kOk;
}

}  // namespace femtolog::logging
