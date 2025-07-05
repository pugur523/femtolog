// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGING_IMPL_BACKEND_WORKER_H_
#define INCLUDE_FEMTOLOG_LOGGING_IMPL_BACKEND_WORKER_H_

#include <atomic>
#include <thread>
#include <vector>

#include "femtolog/base/log_entry.h"
#include "femtolog/base/string_registry.h"
#include "femtolog/logging/impl/spsc_queue.h"
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"

namespace femtolog::logging {

enum class BackendWorkerStatus : uint8_t {
  kUninitialized = 0,
  kIdling = 1,
  kRunning = 2,
  kUnknown = 3,

  // Keep this at the end and equal to the last entry.
  kMaxValue = kUnknown,
};

class BackendWorker {
 public:
  BackendWorker();
  ~BackendWorker();

  BackendWorker(const BackendWorker&) = delete;
  BackendWorker& operator=(const BackendWorker&) = delete;

  BackendWorker(BackendWorker&&) noexcept = delete;
  BackendWorker& operator=(BackendWorker&&) noexcept = delete;

  void init(SpscQueue* queue,
            StringRegistry* string_registry,
            const FemtologOptions& options = FemtologOptions());

  void start();
  void stop();

  void register_sink(std::unique_ptr<SinkBase> sink);
  void clear_sinks();

  BackendWorkerStatus status() const { return status_; }

 private:
  void set_cpu_affinity();
  bool read_and_process_one();
  void run_loop();
  void apply_polling_strategy(bool data_dequeued);

  void flush();
  void process_log_entry(LogEntry* entry);

  alignas(64) std::vector<uint8_t> dequeue_buffer_;
  uint8_t* dequeue_buffer_ptr_ = nullptr;
  std::vector<std::unique_ptr<SinkBase>> sinks_;
  SpscQueue* queue_ = nullptr;
  StringRegistry* string_registry_ = nullptr;
  std::thread worker_thread_;
  std::size_t worker_thread_cpu_affinity_ = 5;
  std::atomic<bool> shutdown_required_{false};
  fmt::basic_memory_buffer<char, 512> format_buffer_;

  // Polling strategy state
  std::size_t idle_iterations_ = 0;

  BackendWorkerStatus status_ = BackendWorkerStatus::kUninitialized;
};

}  // namespace femtolog::logging

#endif  // INCLUDE_FEMTOLOG_LOGGING_IMPL_BACKEND_WORKER_H_
