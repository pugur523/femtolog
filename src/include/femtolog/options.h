// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_OPTIONS_H_
#define INCLUDE_FEMTOLOG_OPTIONS_H_

#include <cstddef>
#include <limits>

namespace femtolog {

/**
 * @brief Configuration options for the femtolog's frontend and backend.
 *
 * This struct defines various parameters to customize femtolog's behavior,
 * including queue sizes and backend thread affinity.
 */
struct FemtologOptions {
  /**
   * @brief Size of the Single-Producer Single-Consumer (SPSC) queue to hold log
   * entries.
   *
   * This queue is used by frontend threads to send log messages to the backend.
   * A larger size can reduce blocking but consumes more memory.
   * Default: 512MiB (1024 * 1024 * 512 bytes)
   */
  std::size_t spsc_queue_size = 1024 * 1024 * 512;

  /**
   * @brief Size of the buffer used by the backend for formatting log messages.
   *
   * This buffer holds formatted log strings before they are written to sinks.
   * Default: 16KiB (1024 * 16 bytes)
   */
  std::size_t backend_format_buffer_size = 1024 * 16;

  /**
   * @brief Size of the buffer used by the backend for dequeuing messages.
   *
   * The backend thread dequeues messages in batches into this buffer for
   * processing.
   * Default: 16KiB (1024 * 16 bytes)
   */
  std::size_t backend_dequeue_buffer_size = 1024 * 16;

  /**
   * @brief CPU core ID for the backend worker thread's affinity.
   *
   * This pins the backend thread to a specific CPU core to improve cache
   * locality and reduce scheduling overhead. Set to a value like
   * `std::numeric_limits<std::size_t>::max()` to disable CPU affinity.
   * Default: std::numeric_limits<std::size_t>::max() (Disabled)
   * Note: CPU affinity is not currently supported on MacOS.
   */
  std::size_t backend_worker_cpu_affinity =
      std::numeric_limits<std::size_t>::max();
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_OPTIONS_H_
