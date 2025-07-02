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
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"
#include "fmt/core.h"
#include "logging/impl/internal_logger.h"

namespace femtolog {

class FEMTOLOG_EXPORT Logger {
  using InternalLogger = logging::InternalLogger;
  using BackendWorker = logging::BackendWorker;

 public:
  ~Logger() = default;

  Logger(const Logger&) = default;
  Logger& operator=(const Logger&) = default;

  Logger(Logger&&) noexcept = default;
  Logger& operator=(Logger&&) noexcept = default;

  inline static Logger& logger() {
    thread_local Logger logger_;
    return logger_;
  }

  inline void init(const FemtologOptions& options = FemtologOptions()) {
    internal_logger().init(options);
  }

  template <typename T,
            typename = typename std::enable_if<
                std::is_base_of<SinkBase, T>::value>::type,
            typename... Args>
  inline void register_sink(Args&&... args) {
    internal_logger().register_sink(
        std::make_unique<T>(std::forward<Args>(args)...));
  }

  inline void clear_sinks() { internal_logger().clear_sinks(); }

  inline void start_worker() { internal_logger().start_worker(); }

  inline void stop_worker() { internal_logger().stop_worker(); }

  template <LogLevel level, FixedString fmt, typename... Args>
  inline constexpr void log(Args&&... args) {
    internal_logger().log<level, fmt, Args...>(std::forward<Args>(args)...);
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

  inline void level(LogLevel level) { internal_logger().level(level); }

  inline void level(const char* level_str) {
    LogLevel new_level = log_level_from_string(level_str);
    if (new_level != LogLevel::kUnknown) {
      level(new_level);
    }
  }

  inline LogLevel level() const { return internal_logger().level(); }

 private:
  Logger() = default;

  inline static InternalLogger& internal_logger() {
    thread_local std::unique_ptr<InternalLogger> internal_logger_ =
        std::make_unique<InternalLogger>();
    return *internal_logger_;
  }
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_LOGGER_H_
