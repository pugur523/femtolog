// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGER_H_
#define INCLUDE_FEMTOLOG_LOGGER_H_

#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#include "femtolog/base/femtolog_export.h"
#include "femtolog/base/format_util.h"
#include "femtolog/core/diagnostics/signal_handler.h"
#include "femtolog/core/diagnostics/stack_trace.h"
#include "femtolog/core/diagnostics/terminate_handler.h"
#include "femtolog/logging/impl/backend_worker.h"
#include "femtolog/logging/impl/internal_logger.h"
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"

namespace femtolog {

class FEMTOLOG_EXPORT Logger {
  using InternalLogger = logging::InternalLogger;
  using BackendWorker = logging::BackendWorker;

 public:
  ~Logger() = default;

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  Logger(Logger&&) noexcept = default;
  Logger& operator=(Logger&&) noexcept = default;

  inline void init(const FemtologOptions& options = FemtologOptions()) {
    internal_logger_->init(options);
    register_exception_handlers();
  }

  template <
      typename T,
      typename = typename std::enable_if_t<std::is_base_of_v<SinkBase, T>>,
      typename... Args>
  inline void register_sink(Args&&... args) {
    internal_logger_->register_sink(
        std::make_unique<T>(std::forward<Args>(args)...));
  }

  inline void clear_sinks() { internal_logger_->clear_sinks(); }

  inline void start_worker() { internal_logger_->start_worker(); }

  inline void stop_worker() { internal_logger_->stop_worker(); }

  template <LogLevel level, FixedString fmt, typename... Args>
  inline constexpr void log(Args&&... args) noexcept {
    internal_logger_->log<level, fmt, false, Args...>(
        std::forward<Args>(args)...);
  }

  template <FixedString fmt, typename... Args>
  inline constexpr void raw(Args&&... args) noexcept {
    log<LogLevel::kRaw, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void fatal(Args&&... args) noexcept {
    log<LogLevel::kFatal, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void error(Args&&... args) noexcept {
    log<LogLevel::kError, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void warn(Args&&... args) noexcept {
    log<LogLevel::kWarn, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void info(Args&&... args) noexcept {
    log<LogLevel::kInfo, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void debug(Args&&... args) noexcept {
    log<LogLevel::kDebug, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void trace(Args&&... args) noexcept {
    log<LogLevel::kTrace, fmt>(std::forward<Args>(args)...);
  }

  template <LogLevel level, FixedString fmt, typename... Args>
  inline constexpr void log_ref(Args&&... args) noexcept {
    internal_logger_->log<level, fmt, true, Args...>(
        std::forward<Args>(args)...);
  }

  template <FixedString fmt, typename... Args>
  inline constexpr void raw_ref(Args&&... args) noexcept {
    static_assert(sizeof...(Args) > 0, "use `raw` instead of `raw_ref`");
    log_ref<LogLevel::kRaw, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void fatal_ref(Args&&... args) noexcept {
    log_ref<LogLevel::kFatal, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void error_ref(Args&&... args) noexcept {
    log_ref<LogLevel::kError, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void warn_ref(Args&&... args) noexcept {
    log_ref<LogLevel::kWarn, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void info_ref(Args&&... args) noexcept {
    log_ref<LogLevel::kInfo, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void debug_ref(Args&&... args) noexcept {
    log_ref<LogLevel::kDebug, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void trace_ref(Args&&... args) noexcept {
    log_ref<LogLevel::kTrace, fmt>(std::forward<Args>(args)...);
  }

  inline void flush() noexcept { internal_logger_->flush(); }

  inline void level(LogLevel level) { internal_logger_->level(level); }

  inline void level(const char* level_str) {
    LogLevel new_level = log_level_from_string(level_str);
    if (new_level != LogLevel::kUnknown) {
      level(new_level);
    }
  }

  inline LogLevel level() const { return internal_logger_->level(); }

  inline std::size_t enqueued_count() const {
    return internal_logger_->enqueued_count();
  }

  inline std::size_t dropped_count() const {
    return internal_logger_->dropped_count();
  }

  inline void reset_count() { internal_logger_->reset_count(); }

  static Logger& logger();

  static Logger& global_logger();

  inline static Logger create_logger() { return Logger(); }

  static bool register_exception_handlers() {
    static const bool initialized = [] {
      core::register_terminate_handler();
      core::register_signal_handlers();
      core::register_stack_trace_handler();
      return true;
    }();
    return initialized;
  }

 private:
  Logger() : internal_logger_(std::make_unique<InternalLogger>()) {}

  std::unique_ptr<InternalLogger> internal_logger_ = nullptr;
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_LOGGER_H_
