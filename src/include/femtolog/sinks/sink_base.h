// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_SINKS_SINK_BASE_H_
#define INCLUDE_FEMTOLOG_SINKS_SINK_BASE_H_

#include <cstdint>
#include <ctime>

#include "core/check.h"
#include "femtolog/base/format_util.h"
#include "femtolog/base/log_entry.h"
#include "fmt/format.h"

namespace femtolog {

enum class TimeZone : uint8_t {
  kUtc = 0,
  kLocal = 1,
};

class SinkBase {
 public:
  SinkBase() = default;
  virtual ~SinkBase() = default;

  SinkBase(const SinkBase&) = delete;
  SinkBase& operator=(const SinkBase&) = delete;

  SinkBase(SinkBase&&) noexcept = default;
  SinkBase& operator=(SinkBase&&) noexcept = default;

  virtual inline void on_log(const LogEntry& entry,
                             const char* content,
                             std::size_t len) = 0;

 protected:
  template <TimeZone tz = TimeZone::kLocal,
            FixedString fmt = "{:%H:%M:%S}.{:09d} ">
  static std::size_t format_timestamp(uint64_t time_ns,
                                      char* buf,
                                      std::size_t buf_size) {
    DCHECK(buf);
    DCHECK_GT(buf_size, 30);

    // separate seconds and nanoseconds
    time_t seconds = static_cast<time_t>(time_ns / 1'000'000'000);
    uint32_t nanoseconds = static_cast<uint32_t>(time_ns % 1'000'000'000);

    // convert seconds to struct tm (thread-safe)
    std::tm tm_buf;
    if constexpr (tz == TimeZone::kUtc) {
#if IS_WINDOWS
      gmtime_s(&tm_buf, &seconds);
#else
      gmtime_r(&seconds, &tm_buf);
#endif
    } else if constexpr (tz == TimeZone::kLocal) {
#if IS_WINDOWS
      localtime_s(&tm_buf, &seconds);
#else
      localtime_r(&seconds, &tm_buf);
#endif
    }

    auto result = fmt::format_to_n(buf, buf_size, FMT_STRING(fmt.data), tm_buf,
                                   nanoseconds);

    if (result.size > buf_size - 1) {
      return result.size;
    }

    buf[result.size] = '\0';
    return result.size;
  }
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_SINKS_SINK_BASE_H_
