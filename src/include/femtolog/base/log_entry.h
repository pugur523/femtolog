// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BASE_LOG_ENTRY_H_
#define INCLUDE_FEMTOLOG_BASE_LOG_ENTRY_H_

#include <algorithm>
#include <string_view>

#include "femtolog/base/log_level.h"

namespace femtolog {

struct LogEntry {
  LogEntry() = default;
  ~LogEntry() = default;

  [[nodiscard]] inline const char* payload() const noexcept {
    return reinterpret_cast<const char*>(this + 1);
  }

  [[nodiscard]] inline char* payload() noexcept {
    return reinterpret_cast<char*>(this + 1);
  }

  [[nodiscard]] inline std::string_view message_view() const noexcept {
    return std::string_view(payload(), payload_len);
  }

  [[nodiscard]] inline constexpr std::size_t total_size() const noexcept {
    return sizeof(LogEntry) + payload_len;
  }

  [[nodiscard]] inline std::size_t copy_raw_payload(
      char* dst_buf,
      std::size_t dst_size) const noexcept {
    std::size_t to_copy = std::min<std::size_t>(dst_size, payload_len);
    std::memcpy(dst_buf, payload(), to_copy);
    return to_copy;
  }

  [[nodiscard]] static LogEntry* create(void* buffer,
                                        uint32_t thread_id,
                                        uint16_t format_id,
                                        LogLevel level,
                                        uint64_t timestamp_ns,
                                        const char* payload,
                                        std::size_t payload_len) {
    auto* entry = reinterpret_cast<LogEntry*>(buffer);
    entry->timestamp_ns = timestamp_ns;
    entry->thread_id = thread_id;
    entry->format_id = format_id;
    entry->payload_len = static_cast<uint16_t>(payload_len);
    entry->level = level;

    std::memcpy(entry->payload(), payload, payload_len);
    return entry;
  }

  uint16_t payload_len = 0;
  uint16_t format_id = 0;
  uint32_t thread_id = 0;
  uint64_t timestamp_ns = 0;
  LogLevel level = LogLevel::kInfo;
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_BASE_LOG_ENTRY_H_
