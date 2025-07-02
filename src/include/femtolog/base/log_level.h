// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BASE_LOG_LEVEL_H_
#define INCLUDE_FEMTOLOG_BASE_LOG_LEVEL_H_

#include <cstdint>

#include "core/base/string_util.h"

namespace femtolog {

enum class LogLevel : uint8_t {
  kFatal = 0,
  kError = 1,
  kWarn = 2,
  kInfo = 3,
  kDebug = 4,
  kTrace = 5,
  kUnknown = 6,

  // Keep this at the end and equal to the last entry.
  kMaxValue = kUnknown,
};

inline constexpr const char* log_level_to_lower_str(LogLevel level) {
  switch (level) {
    case LogLevel::kFatal: return "fatal";
    case LogLevel::kError: return "error";
    case LogLevel::kWarn: return "warn";
    case LogLevel::kInfo: return "info";
    case LogLevel::kDebug: return "debug";
    case LogLevel::kTrace: return "trace";
    default: return "unknown";
  }
}

inline constexpr const char* log_level_to_upper_str(LogLevel level) {
  switch (level) {
    case LogLevel::kFatal: return "FATAL";
    case LogLevel::kError: return "ERROR";
    case LogLevel::kWarn: return "WARN";
    case LogLevel::kInfo: return "INFO";
    case LogLevel::kDebug: return "DEBUG";
    case LogLevel::kTrace: return "TRACE";
    default: return "UNKNOWN";
  }
}

inline constexpr const char* log_level_to_ansi_color(LogLevel level) {
  switch (level) {
    case LogLevel::kFatal: return core::kMagenta;
    case LogLevel::kError: return core::kRed;
    case LogLevel::kWarn: return core::kYellow;
    case LogLevel::kInfo: return core::kGreen;
    case LogLevel::kDebug: return core::kCyan;
    case LogLevel::kTrace: return core::kGray;
    default: return "";
  }
}

template <std::size_t N>
inline constexpr LogLevel log_level_from_string(const char str[N]) {
  constexpr const char* lower_level = core::to_lower_ascii(str);
  constexpr std::string_view level_sv(lower_level);
  if constexpr (level_sv == "fatal") {
    return LogLevel::kFatal;
  } else if constexpr (level_sv == "error") {
    return LogLevel::kError;
  } else if constexpr (level_sv == "warn") {
    return LogLevel::kWarn;
  } else if constexpr (level_sv == "info") {
    return LogLevel::kInfo;
  } else if constexpr (level_sv == "debug") {
    return LogLevel::kDebug;
  } else if constexpr (level_sv == "trace") {
    return LogLevel::kTrace;
  }
  return LogLevel::kUnknown;
}

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_BASE_LOG_LEVEL_H_
