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
  inline constexpr void log(Args&&... args) {
    internal_logger_->log<level, fmt, Args...>(std::forward<Args>(args)...);
  }

  template <FixedString fmt, typename... Args>
  inline constexpr void raw(Args&&... args) {
    log<LogLevel::kRaw, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void fatal(Args&&... args) {
    log<LogLevel::kFatal, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void error(Args&&... args) {
    log<LogLevel::kError, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void warn(Args&&... args) {
    log<LogLevel::kWarn, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void info(Args&&... args) {
    log<LogLevel::kInfo, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void debug(Args&&... args) {
    log<LogLevel::kDebug, fmt>(std::forward<Args>(args)...);
  }
  template <FixedString fmt, typename... Args>
  inline constexpr void trace(Args&&... args) {
    log<LogLevel::kTrace, fmt>(std::forward<Args>(args)...);
  }

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

  inline static Logger& logger() {
    thread_local Logger logger_;
    return logger_;
  }

  inline static Logger& global_logger() {
    static Logger logger_;
    return logger_;
  }

 private:
  Logger() : internal_logger_(std::make_unique<InternalLogger>()) {}

  std::unique_ptr<InternalLogger> internal_logger_ = nullptr;
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_LOGGER_H_
