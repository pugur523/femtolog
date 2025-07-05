// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BASE_LOG_ENTRY_H_
#define INCLUDE_FEMTOLOG_BASE_LOG_ENTRY_H_

#include <algorithm>
#include <string_view>

#include "femtolog/base/log_level.h"

namespace femtolog {

struct alignas(64) LogEntry {
  LogEntry() = default;
  ~LogEntry() = default;

  [[nodiscard]] inline const char* payload() const noexcept {
    return reinterpret_cast<const char*>(this + 1);
  }

  [[nodiscard]] inline char* payload() noexcept {
    return reinterpret_cast<char*>(this + 1);
  }

  [[nodiscard]] inline std::string_view message_view() const noexcept {
    return std::string_view(payload(), content_len);
  }

  [[nodiscard]] inline constexpr std::size_t total_size() const noexcept {
    return sizeof(LogEntry) + payload_size;
  }

  [[nodiscard]] inline constexpr std::size_t aligned_size() const noexcept {
    return align_up(total_size());
  }

  [[nodiscard]] inline std::size_t copy_raw_payload(
      char* dst_buf,
      std::size_t dst_size) const noexcept {
    std::size_t to_copy = std::min<std::size_t>(dst_size, content_len);
    std::memcpy(dst_buf, payload(), to_copy);
    return to_copy;
  }

  [[nodiscard]] static LogEntry* create(void* buffer,
                                        uint32_t thread_id,
                                        uint16_t format_id,
                                        LogLevel level,
                                        uint64_t timestamp_ns,
                                        const char* payload,
                                        std::size_t content_len) {
    auto* entry = reinterpret_cast<LogEntry*>(buffer);
    entry->thread_id = thread_id;
    entry->format_id = format_id;
    entry->level = level;
    entry->content_len = static_cast<uint16_t>(content_len);
    entry->payload_size = static_cast<uint16_t>(sizeof(LogEntry) + content_len);
    entry->timestamp_ns = timestamp_ns;

    std::memcpy(entry->payload(), payload, content_len);

    if (entry->content_len <= 64) [[likely]] {
      // Small message - use direct copy for better performance
      __builtin_memcpy(entry->payload(), payload, content_len);
    } else {
      // Larger message - use memcpy
      std::memcpy(entry->payload(), payload, content_len);
    }
    return entry;
  }

  [[nodiscard]] static constexpr std::size_t align_up(
      std::size_t size) noexcept {
    constexpr const std::size_t kAlignment = alignof(LogEntry);
    return (size + kAlignment - 1) & ~(kAlignment - 1);
  }

  uint32_t thread_id = 0;
  uint16_t format_id = 0;
  LogLevel level = LogLevel::kInfo;
  uint16_t payload_size = 0;
  uint16_t content_len = 0;
  uint64_t timestamp_ns = 0;
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_BASE_LOG_ENTRY_H_
