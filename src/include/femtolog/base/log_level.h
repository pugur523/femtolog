// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BASE_LOG_LEVEL_H_
#define INCLUDE_FEMTOLOG_BASE_LOG_LEVEL_H_

#include <cstdint>
#include <string>

#include "femtolog/core/base/string_util.h"

namespace femtolog {

enum class LogLevel : uint8_t {
  kRaw = 0,
  kFatal = 1,
  kError = 2,
  kWarn = 3,
  kInfo = 4,
  kDebug = 5,
  kTrace = 6,
  kUnknown = 8,

  // Keep this at the end and equal to the last entry.
  kMaxValue = kUnknown,
};

inline constexpr const char* log_level_to_lower_str(LogLevel level) {
  switch (level) {
    case LogLevel::kRaw: return "raw";
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
    case LogLevel::kRaw: return "RAW";
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
    case LogLevel::kRaw: return core::kReset;
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
constexpr bool compare_array_and_cstring(const std::array<char, N>& arr,
                                         const char* cstr) {
  for (std::size_t i = 0; i < arr.size(); ++i) {
    if (arr[i] != cstr[i]) {
      return false;
    }
  }
  return cstr[arr.size()] == '\0';
}

inline constexpr bool cmpstr(const char* a, const char* b) {
  return std::char_traits<char>::compare(
             a, b, std::char_traits<char>::length(b)) == 0;
}

inline constexpr LogLevel log_level_from_string(const char* str) {
  if (cmpstr(str, "raw")) {
    return LogLevel::kRaw;
  } else if (cmpstr(str, "fatal")) {
    return LogLevel::kFatal;
  } else if (cmpstr(str, "error")) {
    return LogLevel::kError;
  } else if (cmpstr(str, "warn")) {
    return LogLevel::kWarn;
  } else if (cmpstr(str, "info")) {
    return LogLevel::kInfo;
  } else if (cmpstr(str, "debug")) {
    return LogLevel::kDebug;
  } else if (cmpstr(str, "trace")) {
    return LogLevel::kTrace;
  } else {
    return LogLevel::kUnknown;
  }
}

inline constexpr std::size_t level_len(LogLevel lvl) {
  return std::char_traits<char>::length(log_level_to_lower_str(lvl));
}

inline constexpr std::size_t level_ansi_len(LogLevel lvl) {
  return std::char_traits<char>::length(log_level_to_ansi_color(lvl));
}

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_BASE_LOG_LEVEL_H_
