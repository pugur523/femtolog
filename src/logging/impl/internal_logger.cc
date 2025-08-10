// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/internal_logger.h"

#include <utility>

#include "femtolog/logging/impl/backend_worker.h"
#include "femtolog/options.h"

namespace femtolog::logging {

InternalLogger::InternalLogger() : thread_id_(current_thread_id()) {}

InternalLogger::~InternalLogger() {
  if (backend_worker_.status() == BackendWorkerStatus::kRunning) {
    stop_worker();
  }
}

void InternalLogger::init(const FemtologOptions& options) {
  FEMTOLOG_DCHECK_GT(options.spsc_queue_size, 0);
  queue_.reserve(options.spsc_queue_size);
  FEMTOLOG_DCHECK_GE(queue_.capacity(), options.spsc_queue_size);

  if (backend_worker_.status() == BackendWorkerStatus::kUninitialized)
      [[likely]] {
    backend_worker_.init(&queue_, options);
  }
}

void InternalLogger::register_sink(std::unique_ptr<SinkBase> sink) {
  backend_worker_.register_sink(std::move(sink));
}

void InternalLogger::clear_sinks() {
  backend_worker_.clear_sinks();
}

void InternalLogger::start_worker() {
  backend_worker_.start();
}

void InternalLogger::stop_worker() {
  backend_worker_.stop();
}

// static
uint32_t InternalLogger::current_thread_id() noexcept {
  static thread_local uint32_t cached_id = []() noexcept -> uint32_t {
    const auto tid = std::this_thread::get_id();
    const auto hash_val = std::hash<std::thread::id>{}(tid);

    // Ensure non-zero ID for better performance characteristics
    return static_cast<uint32_t>(hash_val) | 1;
  }();

  return cached_id;
}

// static
bool InternalLogger::is_ansi_sequence_available() {
  static const bool result = [] {
#if FEMTOLOG_IS_WINDOWS
    // check if GetStdHandle() returns terminal handle
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
      return false;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
      return false;
    }

    // enable virtual terminal processing
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
      // cannot use ansi sequence if fail to enable virtual terminal
      return false;
    }
    return true;
#else
    return isatty(STDOUT_FILENO);
#endif
  }();

  return result;
}

}  // namespace femtolog::logging
