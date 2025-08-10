// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_SINKS_STDOUT_SINK_H_
#define INCLUDE_FEMTOLOG_SINKS_STDOUT_SINK_H_

#include <cstring>
#include <memory>
#include <mutex>

#include "femtolog/base/log_entry.h"
#include "femtolog/base/log_level.h"
#include "femtolog/build/build_flag.h"
#include "femtolog/logging/impl/internal_logger.h"
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"

#if FEMTOLOG_IS_WINDOWS
#include <io.h>
#else
#include <sys/uio.h>
#include <unistd.h>
#endif

namespace femtolog {

template <bool enable_buffering = false, bool sync_write = true>
class StdoutSink final : public SinkBase {
 public:
  explicit StdoutSink(ColorMode mode = ColorMode::kAuto) : mode_(mode) {
    if constexpr (enable_buffering) {
      buffer_ = std::make_unique<char[]>(kBufferCapacity);
    }
  }

  ~StdoutSink() override {
    if constexpr (enable_buffering) {
      flush();
    }
  }

  inline void on_log(const LogEntry& entry,
                     const char* content,
                     std::size_t len) override {
    if constexpr (!enable_buffering) {
      write_direct(entry.level, content, len);
      return;
    }
    const std::size_t total_len = estimate_length(entry.level, len);
    if (cursor_ + total_len > kBufferCapacity) {
      flush();
    }
    if (total_len > kBufferCapacity) {
      write_direct(entry.level, content, len);
      return;
    }
    cursor_ +=
        build_message(buffer_.get() + cursor_, entry.level, content, len);
  }

 private:
  inline std::size_t estimate_length(LogLevel level,
                                     std::size_t content_len) const {
    std::size_t len = content_len + kSepLen;
    if (is_color_enabled()) {
      len += kAnsiStyleLen + kResetLen + kAnsiStyleLen;
    }
    len += level_len(level);
    return len;
  }

  inline std::size_t build_message(char* out,
                                   LogLevel level,
                                   const char* content,
                                   std::size_t content_len) const {
    char* p = out;
    if (is_color_enabled()) {
      std::memcpy(p, kBold, kAnsiStyleLen);
      p += kAnsiStyleLen;
      const char* col = log_level_to_ansi_color(level);
      std::memcpy(p, col, kAnsiStyleLen);
      p += kAnsiStyleLen;
    }
    if (level != LogLevel::kRaw) {
      const char* lvl = log_level_to_lower_str(level);
      std::size_t lvl_len = level_len(level);
      std::memcpy(p, lvl, lvl_len);
      p += lvl_len;
      std::memcpy(p, kSep, kSepLen);
      p += kSepLen;
    }
    if (is_color_enabled()) {
      std::memcpy(p, kReset, kResetLen);
      p += kResetLen;
    }
    std::memcpy(p, content, content_len);
    p += content_len;
    return p - out;
  }

  inline void flush() {
    if constexpr (enable_buffering) {
      if (cursor_ == 0) {
        return;
      }
      lock();
#if FEMTOLOG_IS_WINDOWS
      _write(kStdOutFd, buffer_.get(), static_cast<unsigned int>(cursor_));
#else
      const auto _ = write(kStdOutFd, buffer_.get(), cursor_);
#endif
      unlock();
      cursor_ = 0;
    }
  }

  inline void write_direct(LogLevel level,
                           const char* content,
                           std::size_t len) {
    char tmp[kBufferCapacity];
    std::size_t total = build_message(tmp, level, content, len);
    lock();
#if FEMTOLOG_IS_WINDOWS
    _write(kStdOutFd, tmp, static_cast<unsigned int>(total));
#else
    const auto _ = write(kStdOutFd, tmp, total);
#endif
    unlock();
  }

  inline static void lock() {
    if constexpr (sync_write) {
      stdout_mutex().lock();
    }
  }
  inline static void unlock() {
    if constexpr (sync_write) {
      stdout_mutex().unlock();
    }
  }

  inline bool is_color_enabled() const {
    return (mode_ == ColorMode::kAuto &&
            logging::InternalLogger::is_ansi_sequence_available()) ||
           mode_ == ColorMode::kAlways;
  }

  static std::mutex& stdout_mutex() {
    static std::mutex m;
    return m;
  }

  std::unique_ptr<char[]> buffer_;
  std::size_t cursor_ = 0;
  ColorMode mode_;

  static constexpr int kStdOutFd = 1;
  static constexpr std::size_t kBufferCapacity = 4096;
  static constexpr const char* kSep = ": ";
  static constexpr std::size_t kSepLen = 2;
  static constexpr const char* kBold = core::kBold;
  static constexpr std::size_t kAnsiStyleLen = 4;
  static constexpr const char* kReset = core::kReset;
  static constexpr std::size_t kResetLen = 4;
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_SINKS_STDOUT_SINK_H_
