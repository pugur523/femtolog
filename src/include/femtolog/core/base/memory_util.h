// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_CORE_BASE_MEMORY_UTIL_H_
#define INCLUDE_FEMTOLOG_CORE_BASE_MEMORY_UTIL_H_

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/core_export.h"

#if FEMTOLOG_IS_WINDOWS
#include <windows.h>
#elif FEMTOLOG_IS_LINUX
#include <sys/mman.h>
#endif

namespace core {

// constexpr std::size_t kPrefetchThreshold = 256;

// Optimized memory allocation with proper alignment
inline void* aligned_alloc_wrapper(std::size_t alignment, std::size_t size) {
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

inline void aligned_free_wrapper(void* ptr) {
#if FEMTOLOG_IS_WINDOWS
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}

struct AlignedDeleter {
  inline void operator()(void* ptr) const { aligned_free_wrapper(ptr); }
};

}  // namespace core

#endif  // INCLUDE_FEMTOLOG_CORE_BASE_MEMORY_UTIL_H_

