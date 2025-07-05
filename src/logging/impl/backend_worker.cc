// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/backend_worker.h"

#include <iostream>
#include <limits>
#include <utility>
#include <vector>

#include "femtolog/base/string_registry.h"
#include "femtolog/build/build_flag.h"
#include "femtolog/core/check.h"
#include "femtolog/logging/impl/args_deserializer.h"
#include "femtolog/logging/impl/internal_logger.h"
#include "femtolog/options.h"
#include "fmt/args.h"
#include "fmt/core.h"
#include "fmt/format.h"

#if FEMTOLOG_ENABLE_AVX2
#include <immintrin.h>
#endif

#if FEMTOLOG_IS_WINDOWS
#include <windows.h>
#elif FEMTOLOG_IS_LINUX
#include <pthread.h>
#include <sched.h>  // for CPU_ZERO, CPU_SET
#endif

namespace femtolog::logging {

BackendWorker::BackendWorker() = default;

BackendWorker::~BackendWorker() {
  if (status_ == BackendWorkerStatus::kRunning) {
    stop();
  }
}

void BackendWorker::init(SpscQueue* queue,
                         StringRegistry* string_registry,
                         const FemtologOptions& options) {
  FEMTOLOG_DCHECK_EQ(status_, BackendWorkerStatus::kUninitialized);
  FEMTOLOG_DCHECK_GT(options.backend_dequeue_buffer_size, 0);
  FEMTOLOG_DCHECK_GT(options.backend_format_buffer_size, 0);
  FEMTOLOG_DCHECK(queue);
  FEMTOLOG_DCHECK(string_registry);

  queue_ = queue;
  string_registry_ = string_registry;
  status_ = BackendWorkerStatus::kIdling;
  dequeue_buffer_.reserve(options.backend_dequeue_buffer_size);
  dequeue_buffer_.resize(options.backend_dequeue_buffer_size);
  format_buffer_.reserve(options.backend_format_buffer_size);
  dequeue_buffer_ptr_ = dequeue_buffer_.data();
  worker_thread_cpu_affinity_ = options.backend_worker_cpu_affinity;
}

// Using std::jthread for automatic joining in C++20 is preferred,
// but std::thread is used for broader compatibility here.
void BackendWorker::start() {
  FEMTOLOG_DCHECK_EQ(status_, BackendWorkerStatus::kIdling);
  FEMTOLOG_DCHECK_GT(sinks_.size(), 0);
  FEMTOLOG_DCHECK(dequeue_buffer_ptr_);
  worker_thread_ = std::thread(&BackendWorker::run_loop, this);

  set_cpu_affinity();

  status_ = BackendWorkerStatus::kRunning;
}

void BackendWorker::stop() {
  FEMTOLOG_DCHECK_EQ(status_, BackendWorkerStatus::kRunning);
  shutdown_required_.store(true, std::memory_order_release);
  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
  status_ = BackendWorkerStatus::kIdling;
}

void BackendWorker::register_sink(std::unique_ptr<SinkBase> sink) {
  FEMTOLOG_DCHECK_NE(status_, BackendWorkerStatus::kRunning)
      << "attempted to register new sink while running.";
  FEMTOLOG_DCHECK_EQ(status_, BackendWorkerStatus::kIdling);
  FEMTOLOG_DCHECK(sink);

  sinks_.push_back(std::move(sink));
}

void BackendWorker::clear_sinks() {
  FEMTOLOG_DCHECK_NE(status_, BackendWorkerStatus::kRunning)
      << "attempted to clear all sinks while running.";
  FEMTOLOG_DCHECK_EQ(status_, BackendWorkerStatus::kIdling);
  sinks_.clear();
}

void BackendWorker::set_cpu_affinity() {
  if (worker_thread_cpu_affinity_ == std::numeric_limits<std::size_t>::max()) {
    // Affinity is disabled
    return;
  }
#if FEMTOLOG_IS_LINUX
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(worker_thread_cpu_affinity_, &cpuset);
  int rc = pthread_setaffinity_np(worker_thread_.native_handle(),
                                  sizeof(cpu_set_t), &cpuset);
  if (rc != 0) {
    std::cerr << "Failed to set thread affinity to CPU "
              << worker_thread_cpu_affinity_ << " (errno=" << errno << ")\n";
  }
#elif FEMTOLOG_IS_WINDOWS
  DWORD_PTR mask = 1ull << worker_thread_cpu_affinity_;
  HANDLE handle = static_cast<HANDLE>(worker_thread_.native_handle());
  DWORD_PTR result = SetThreadAffinityMask(handle, mask);
  if (result == 0) {
    std::cerr << "Failed to set thread affinity to CPU "
              << worker_thread_cpu_affinity_
              << " (GetLastError=" << GetLastError() << ")\n";
  }
#endif
}

bool BackendWorker::read_and_process_one() {
  LogEntry header_tmp;
  if (queue_->peek_bytes(&header_tmp, sizeof(LogEntry)) !=
      SpscQueueStatus::kOk) {
    return false;
  }

  const std::size_t total_size = header_tmp.aligned_size();
  if (queue_->size() < total_size) {
    return false;
  }
  if (queue_->dequeue_bytes(dequeue_buffer_ptr_, total_size) !=
      SpscQueueStatus::kOk) {
    return false;
  }
  LogEntry* entry = reinterpret_cast<LogEntry*>(dequeue_buffer_ptr_);
  process_log_entry(entry);
  return true;
}

void BackendWorker::run_loop() {
  bool data_dequeued_this_iteration = false;

  // Try to dequeue a log entry while running
  while (!shutdown_required_.load(std::memory_order_acquire)) {
    data_dequeued_this_iteration = read_and_process_one();
    apply_polling_strategy(data_dequeued_this_iteration);
  }
  flush();
}

void BackendWorker::apply_polling_strategy(bool data_dequeued) {
  // Reset counter on successful dequeue
  if (data_dequeued) {
    idle_iterations_ = 0;
    return;
  }

  idle_iterations_++;

  // Tiered backoff strategy
  if (idle_iterations_ <= 2048) {
    // Tier 1: __mm_pause or yield cpu to other threads on same core
#if FEMTOLOG_ENABLE_AVX2
    _mm_pause();
#else
    std::this_thread::yield();
#endif
    return;
  } else if (idle_iterations_ <= 4096) {
    // Tier 2: Short sleep
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    return;
  } else if (idle_iterations_ <= 8192) {
    // Tier 3: Medium sleep
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    return;
  } else if (idle_iterations_ <= 16384) {
    // Tier 4: Long sleep
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    return;
  } else if (idle_iterations_ <= 32768) {
    // Tier 5: Very Long sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return;
  } else if (idle_iterations_ <= 65536) {
    // Tier 5: Super Long sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return;
  }
}

void BackendWorker::flush() {
  while (read_and_process_one()) {}
}

void BackendWorker::process_log_entry(LogEntry* entry) {
  FEMTOLOG_DCHECK(!!entry);
  entry->timestamp_ns = timestamp_ns();

  uint16_t format_id = entry->format_id;

  if (format_id == kLiteralLogStringId) {
    constexpr const std::size_t kBufSize = kMaxPayloadSize;
    char buf[kBufSize];
    std::size_t n = entry->copy_raw_payload(buf, kBufSize);
    buf[n] = '\0';
    for (const auto& sink : sinks_) {
      FEMTOLOG_DCHECK(!!sink);
      sink->on_log(*entry, buf, n);
    }
  } else {
    std::string_view format_str = string_registry_->get_string(format_id);

    ArgsDeserializer deserializer(entry->payload(), string_registry_);

    fmt::dynamic_format_arg_store<fmt::format_context> deserialized_args =
        deserializer.deserialize();

    format_buffer_.resize(0);
    auto result =
        fmt::vformat_to_n(format_buffer_.data(), format_buffer_.capacity(),
                          format_str, deserialized_args);
    for (const auto& sink : sinks_) {
      FEMTOLOG_DCHECK(!!sink);
      sink->on_log(*entry, format_buffer_.data(), result.size);
    }
  }
}

}  // namespace femtolog::logging

