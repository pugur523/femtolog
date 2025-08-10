// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_OPTIONS_H_
#define INCLUDE_FEMTOLOG_OPTIONS_H_

#include <cstddef>
#include <limits>

namespace femtolog {

enum class ColorMode : uint8_t {
  kAuto = 0,
  kAlways = 1,
  kNever = 2,
};

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
   * Default: 8KiB (1024 * 8 bytes)
   */
  std::size_t spsc_queue_size = 1024 * 8;

  /**
   * @brief Size of the buffer used by the backend for formatting log messages.
   *
   * This buffer holds formatted log strings before they are written to sinks.
   * Default: 2KiB (1024 * 2 bytes)
   */
  std::size_t backend_format_buffer_size = 1024 * 2;

  /**
   * @brief Size of the buffer used by the backend for dequeuing messages.
   *
   * The backend thread dequeues messages in batches into this buffer for
   * processing.
   * Default: 4KiB (1024 * 4 bytes)
   */
  std::size_t backend_dequeue_buffer_size = 1024 * 4;

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

  /**
   * @brief When to enable ANSI color sequence.
   *
   * ColorMode::kAuto: detect automatically if the ANSI escape sequence is
   * available. ColorMode::kAlways: always use ANSI escape sequence.
   * ColorMode::kNever: never use ANSI escape sequence.
   */
  ColorMode color_mode = ColorMode::kAuto;

  /**
   * @brief Whether terminate or not on Logger::fatal call
   * If true, terminates the whole program when the backend worker receives log
   * entry whose level is LogLevel::kFatal.
   */
  bool terminate_on_fatal : 1 = true;
};

constexpr FemtologOptions kFastOptions{
    1024 * 1024 * 4, 1024 * 64, 1024 * 64, 5, ColorMode::kAuto,
};

constexpr FemtologOptions kMemorySavingOptions{
    1024, 256, 512, std::numeric_limits<std::size_t>::max(), ColorMode::kAuto,
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_OPTIONS_H_
