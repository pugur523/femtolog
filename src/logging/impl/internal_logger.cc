// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "logging/impl/internal_logger.h"

#include <utility>

#include "femtolog/options.h"
#include "logging/impl/backend_worker.h"

namespace femtolog::logging {

InternalLogger::InternalLogger()
    : thread_id_(current_thread_id()), serializer_() {}

InternalLogger::~InternalLogger() {
  if (backend_worker_.status() == BackendWorkerStatus::kRunning) {
    stop_worker();
  }
}

void InternalLogger::init(const FemtologOptions& options) {
  DCHECK_GT(options.spsc_queue_size, 0);
  queue_.reserve(options.spsc_queue_size);
  DCHECK_GE(queue_.capacity(), options.spsc_queue_size);

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

bool InternalLogger::enqueue_log_entry(const LogEntry* entry) noexcept {
  const std::size_t entry_size = entry->aligned_size();

  // Direct enqueue with minimal overhead
  const SpscQueueStatus result = queue_.enqueue_bytes(entry, entry_size);
  if (result == SpscQueueStatus::kOk) [[likely]] {
    enqueued_count_++;
    return true;
  } else {
    dropped_count_++;
    return false;
  }
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

}  // namespace femtolog::logging
