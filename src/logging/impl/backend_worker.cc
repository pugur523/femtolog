// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "logging/impl/backend_worker.h"

#include <utility>
#include <vector>

#include "core/check.h"
#include "femtolog/base/format_string_registry.h"
#include "femtolog/options.h"
#include "fmt/args.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "logging/impl/args_deserializer.h"
#include "logging/impl/internal_logger.h"

namespace femtolog::logging {

BackendWorker::BackendWorker() = default;

BackendWorker::~BackendWorker() {
  if (status_ == BackendWorkerStatus::kRunning) {
    stop();
  }
}

void BackendWorker::init(SpscQueue* queue, const FemtologOptions& options) {
  DCHECK_EQ(status_, BackendWorkerStatus::kUninitialized);
  DCHECK_GT(options.spsc_queue_size, 0);
  DCHECK_GT(options.backend_dequeue_buffer_size, 0);
  DCHECK_GT(options.format_buffer_size, 0);

  queue_ = queue;
  status_ = BackendWorkerStatus::kIdling;
  dequeue_buffer_.reserve(options.backend_dequeue_buffer_size);
  dequeue_buffer_.resize(options.backend_dequeue_buffer_size);
  format_buffer_.reserve(options.format_buffer_size);
  dequeue_buffer_ptr_ = dequeue_buffer_.data();
}

// Using std::jthread for automatic joining in C++20 is preferred,
// but std::thread is used for broader compatibility here.
void BackendWorker::start() {
  DCHECK_EQ(status_, BackendWorkerStatus::kIdling);
  DCHECK_GT(sinks_.size(), 0);
  DCHECK(dequeue_buffer_ptr_);
  worker_thread_ = std::thread(&BackendWorker::run_loop, this);
  status_ = BackendWorkerStatus::kRunning;
}

void BackendWorker::stop() {
  DCHECK_EQ(status_, BackendWorkerStatus::kRunning);
  shutdown_required_.store(true, std::memory_order_release);
  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
  status_ = BackendWorkerStatus::kIdling;
}

void BackendWorker::register_sink(std::unique_ptr<SinkBase> sink) {
  DCHECK_NE(status_, BackendWorkerStatus::kRunning)
      << "attempted to register new sink while running.";
  DCHECK_EQ(status_, BackendWorkerStatus::kIdling);
  DCHECK(sink);

  sinks_.push_back(std::move(sink));
}

void BackendWorker::clear_sinks() {
  DCHECK_NE(status_, BackendWorkerStatus::kRunning)
      << "attempted to clear all sinks while running.";
  DCHECK_EQ(status_, BackendWorkerStatus::kIdling);
  sinks_.clear();
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
  if (idle_iterations_ <= 8192) {
    // Tier 1: Busy-wait (no sleep, no yield) for very frequent logs
    return;
  } else if (idle_iterations_ <= 16384) {
    // Tier 2: Yield CPU to other threads on same core
    std::this_thread::yield();
    return;
  } else if (idle_iterations_ <= 32768) {
    // Tier 3: Short sleep
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    return;
  } else if (idle_iterations_ <= 65536) {
    // Tier 4: Medium sleep
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    return;
  } else if (idle_iterations_ <= 131072) {
    // Tier 5: Long sleep
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    return;
  } else {
    // Tier 6: Very Long sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return;
  }
  // TODO: Consider using _mm_pause for x86 in busy-wait for power saving
}

void BackendWorker::flush() {
  while (read_and_process_one()) {}
}

void BackendWorker::process_log_entry(LogEntry* entry) {
  DCHECK(!!entry);
  entry->timestamp_ns = timestamp_ns();

  uint16_t format_id = entry->format_id;

  if (format_id == kMaxFormatId) {
    constexpr const std::size_t kBufSize = kMaxPayloadSize;
    char buf[kBufSize];
    std::size_t n = entry->copy_raw_payload(buf, kBufSize);
    buf[n] = '\0';
    for (const auto& sink : sinks_) {
      DCHECK(!!sink);
      sink->on_log(*entry, buf, n);
    }
  } else {
    std::string_view format_str = get_format_string(format_id);

    ArgsDeserializer deserializer(entry->payload());

    fmt::dynamic_format_arg_store<fmt::format_context> deserialized_args =
        deserializer.deserialize();

    format_buffer_.resize(0);
    auto result =
        fmt::vformat_to_n(format_buffer_.data(), format_buffer_.capacity(),
                          format_str, deserialized_args);
    for (const auto& sink : sinks_) {
      DCHECK(!!sink);
      sink->on_log(*entry, format_buffer_.data(), result.size);
    }
  }
}

}  // namespace femtolog::logging

