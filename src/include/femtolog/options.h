// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_OPTIONS_H_
#define INCLUDE_FEMTOLOG_OPTIONS_H_

#include <cstddef>

namespace femtolog {

struct FemtologOptions {
  std::size_t spsc_queue_size = 2 * 1024 * 1024;
  std::size_t format_buffer_size = 512;
  std::size_t backend_dequeue_buffer_size = 4096;
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_OPTIONS_H_
