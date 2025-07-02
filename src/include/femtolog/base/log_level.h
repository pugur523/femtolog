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
constexpr bool compare_array_and_cstring(const std::array<char, N>& arr,
                                         const char* cstr) {
  for (std::size_t i = 0; i < arr.size(); ++i) {
    if (arr[i] != cstr[i]) {
      return false;
    }
  }
  return cstr[arr.size()] == '\0';
}

inline LogLevel log_level_from_string(const char* str) {
  if (std::strcmp(str, "fatal") == 0) {
    return LogLevel::kFatal;
  } else if (std::strcmp(str, "error") == 0) {
    return LogLevel::kError;
  } else if (std::strcmp(str, "warn") == 0) {
    return LogLevel::kWarn;
  } else if (std::strcmp(str, "info") == 0) {
    return LogLevel::kInfo;
  } else if (std::strcmp(str, "debug") == 0) {
    return LogLevel::kDebug;
  } else if (std::strcmp(str, "trace") == 0) {
    return LogLevel::kTrace;
  }
  return LogLevel::kUnknown;
}

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_BASE_LOG_LEVEL_H_
