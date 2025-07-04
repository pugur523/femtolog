// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGING_IMPL_INTERNAL_LOGGER_H_
#define INCLUDE_FEMTOLOG_LOGGING_IMPL_INTERNAL_LOGGER_H_

#include <memory>
#include <utility>

#include "femtolog/base/format_string_registry.h"
#include "femtolog/base/log_entry.h"
#include "femtolog/logging/base/logging_export.h"
#include "femtolog/logging/impl/args_serializer.h"
#include "femtolog/logging/impl/backend_worker.h"
#include "femtolog/logging/impl/spsc_queue.h"
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"

namespace femtolog::logging {

// 4KiB max per entry. (consider sizeof LogEntry)
static constexpr const std::size_t kMaxPayloadSize = 4096 - sizeof(LogEntry);

class FEMTOLOG_LOGGING_EXPORT InternalLogger {
 public:
  InternalLogger();

  ~InternalLogger();

  InternalLogger(const InternalLogger&) = delete;
  InternalLogger& operator=(const InternalLogger&) = delete;

  InternalLogger(InternalLogger&&) noexcept = delete;
  InternalLogger& operator=(InternalLogger&&) noexcept = delete;

  void init(const FemtologOptions& options = FemtologOptions());

  void register_sink(std::unique_ptr<SinkBase> sink);

  void clear_sinks();

  void start_worker();

  void stop_worker();

  [[nodiscard]] SpscQueue& queue() noexcept { return queue_; }
  [[nodiscard]] const SpscQueue& queue() const noexcept { return queue_; }

  [[nodiscard]] inline uint32_t thread_id() const noexcept {
    return thread_id_;
  }

  [[nodiscard]] inline uint64_t enqueued_count() const noexcept {
    return enqueued_count_;
  }

  [[nodiscard]] inline uint64_t dropped_count() const noexcept {
    return dropped_count_;
  }

  template <LogLevel level, FixedString fmt, typename... Args>
  inline void log(Args&&... args) noexcept {
    DCHECK_EQ(backend_worker_.status(), BackendWorkerStatus::kRunning);

    // Compile-time level check
    // Assuming `info` is common threshold
    if constexpr (level > LogLevel::kInfo) {
      if (level > level_) [[unlikely]] {
        return;
      }
    } else {
      if (level > level_) [[likely]] {
        return;
      }
    }

    if constexpr (sizeof...(Args) == 0) {
      constexpr std::string_view view(fmt.data, fmt.size);
      log_literal<level>(view);
    } else {
      constexpr uint16_t format_id = get_format_id<fmt>();
      register_format<fmt>();
      SerializedArgs serialized_args =
          serializer_.serialize(std::forward<Args>(args)...);
      log_serialized<level>(format_id, serialized_args);
    }
  }

  inline void level(LogLevel level) noexcept { level_ = level; }

  [[nodiscard]] inline LogLevel level() const noexcept { return level_; }

  // Assign uint16 max value for literal string log
  static constexpr uint16_t kLiteralLogFormatId = 65535;

 private:
  // Hot data - frequently accessed (first cache line)
  alignas(128) LogLevel level_ = LogLevel::kInfo;
  const uint32_t thread_id_;
  uint64_t enqueued_count_ = 0;
  uint64_t dropped_count_ = 0;
  ArgsSerializer<> serializer_;

  // Cold data - less frequently accessed (separate cache line)
  alignas(128) SpscQueue queue_;

  // Buffer management (separate cache line)
  alignas(LogEntry) alignas(128) alignas(8) uint8_t
      entry_buffer_[sizeof(LogEntry) + kMaxPayloadSize];

  BackendWorker backend_worker_;

  template <LogLevel level>
  inline void log_literal(std::string_view message) {
    const std::size_t payload_len = message.size();
    if (payload_len >= kMaxPayloadSize) [[unlikely]] {
      dropped_count_++;
      return;
    }

    LogEntry* entry = LogEntry::create(entry_buffer_, thread_id_,
                                       kLiteralLogFormatId, level, 0, message);

    enqueue_log_entry(entry);
  }

  template <LogLevel level>
  inline void log_serialized(uint16_t format_id,
                             const SerializedArgs<>& serialized) {
    if (serialized.size() >= kMaxPayloadSize) [[unlikely]] {
      dropped_count_++;
      return;
    }

    LogEntry* entry = LogEntry::create(
        entry_buffer_, thread_id_, format_id, level, 0,
        std::string_view(serialized.data(), serialized.size()));

    enqueue_log_entry(entry);
  }

  void enqueue_log_entry(const LogEntry* entry) noexcept;

  [[nodiscard]] static uint32_t current_thread_id() noexcept;

  friend void internal_logger_enqueue_log_entry_bench(InternalLogger* logger);
};

}  // namespace femtolog::logging

#endif  // INCLUDE_FEMTOLOG_LOGGING_IMPL_INTERNAL_LOGGER_H_
