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
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"

#if FEMTOLOG_IS_WINDOWS
#include <io.h>
#include <windows.h>
#else
#include <sys/uio.h>
#include <unistd.h>
#endif

namespace femtolog {

template <bool enable_buffering = false, bool sync_write = true>
class StdoutSink final : public SinkBase {
 public:
  explicit StdoutSink(ColorMode mode = ColorMode::kAuto) : mode_(mode) {
#if FEMTOLOG_IS_WINDOWS
    if (is_color_enabled()) {
      static bool initialized = [] {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
          SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
        return true;
      }();
      (void)initialized;
    }
#endif

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
      direct_write(entry.level, content, len);
      return;
    }

    const std::size_t total_approximated = kSepLen + kResetLen + len;
    if (cursor_ + total_approximated > kBufferCapacity) {
      flush();
    }

    if (total_approximated > kBufferCapacity) {
      direct_write(entry.level, content, len);
      return;
    }

    const char* level_str = log_level_to_lower_str(entry.level);
    std::size_t level_len = core::safe_strlen(level_str);

    if (is_color_enabled()) {
      std::memcpy(buffer_.get() + cursor_, core::kBold, kBoldLen);
      cursor_ += kBoldLen;

      const char* color = log_level_to_ansi_color(entry.level);
      std::size_t color_len = core::safe_strlen(color);
      std::memcpy(buffer_.get() + cursor_, color, color_len);
      cursor_ += color_len;
    }
    std::memcpy(buffer_.get() + cursor_, level_str, level_len);
    cursor_ += level_len;
    std::memcpy(buffer_.get() + cursor_, kSep, kSepLen);
    if (is_color_enabled()) {
      std::memcpy(buffer_.get() + cursor_, kReset, kResetLen);
      cursor_ += kResetLen;
    }
    cursor_ += kSepLen;
    std::memcpy(buffer_.get() + cursor_, content, len);
    cursor_ += len;
  }

 private:
  inline void flush() {
    if constexpr (enable_buffering) {
      if (cursor_ == 0 || !buffer_) {
        return;
      }
      lock();
#if FEMTOLOG_IS_WINDOWS
      _write(kStdOutFd, buffer_.get(), static_cast<unsigned int>(cursor_));
#else
      [[maybe_unused]] ssize_t written =
          write(kStdOutFd, buffer_.get(), cursor_);
#endif
      unlock();
      cursor_ = 0;
    }
  }

  inline void direct_write(LogLevel level,
                           const char* content,
                           std::size_t content_len) {
    const char* level_str = log_level_to_lower_str(level);
    std::size_t level_len = core::safe_strlen(level_str);
#if FEMTOLOG_IS_WINDOWS
    constexpr std::size_t kMaxStackBuffer = 4096;

    if constexpr (is_color_enabled()) {
      const char* color = log_level_to_ansi_color(level);
      std::size_t color_len = core::safe_strlen(color);

      std::size_t total =
          color_len + kBoldLen + level_len + kSepLen + kResetLen + content_len;

      if (total <= kMaxStackBuffer) {
        char tmp[kMaxStackBuffer];
        char* cursor = tmp;
        std::memcpy(cursor, kBold, kBoldLen);
        cursor += kBoldLen;
        std::memcpy(cursor, color, color_len);
        cursor += color_len;
        std::memcpy(cursor, level_str, level_len);
        cursor += level_len;
        std::memcpy(cursor, kSep, kSepLen);
        cursor += kSepLen;
        std::memcpy(cursor, kReset, kResetLen);
        cursor += kResetLen;
        std::memcpy(cursor, content, content_len);
        cursor += content_len;
        // *cursor = '\0';
        lock();
        _write(kStdOutFd, tmp, static_cast<unsigned int>(total));
        unlock();
        return;
      }

      // fallback: write in chunks
      lock();
      _write(kStdOutFd, kBold, static_cast<unsigned int>(kBoldLen));
      _write(kStdOutFd, color, static_cast<unsigned int>(color_len));
      _write(kStdOutFd, level_str, static_cast<unsigned int>(level_len));
      _write(kStdOutFd, kSep, static_cast<unsigned int>(kSepLen));
      _write(kStdOutFd, kReset, static_cast<unsigned int>(kResetLen));
      _write(kStdOutFd, content, static_cast<unsigned int>(content_len));
      unlock();
    } else {
      std::size_t total = level_len + kSepLen + content_len;

      if (total <= kMaxStackBuffer) {
        char tmp[kMaxStackBuffer];
        char* cursor = tmp;
        std::memcpy(cursor, level_str, level_len);
        cursor += level_len;
        std::memcpy(cursor, kSep, kSepLen);
        cursor += kSepLen;
        std::memcpy(cursor, content, content_len);
        cursor += content_len;
        // *cursor = '\0';
        lock();
        _write(kStdOutFd, tmp, static_cast<unsigned int>(total));
        unlock();
      } else {
        lock();
        _write(kStdOutFd, level_str, static_cast<unsigned int>(level_len));
        _write(kStdOutFd, kSep, static_cast<unsigned int>(kSepLen));
        _write(kStdOutFd, content, static_cast<unsigned int>(content_len));
        unlock();
      }
    }
#else
    if (is_color_enabled()) {
      const char* color = log_level_to_ansi_color(level);
      std::size_t color_len = core::safe_strlen(color);
      struct iovec iov[6];
      iov[0].iov_base = const_cast<char*>(kBold);
      iov[0].iov_len = kBoldLen;
      iov[1].iov_base = const_cast<char*>(color);
      iov[1].iov_len = color_len;
      iov[2].iov_base = const_cast<char*>(level_str);
      iov[2].iov_len = level_len;
      iov[3].iov_base = const_cast<char*>(kReset);
      iov[3].iov_len = kResetLen;
      iov[4].iov_base = const_cast<char*>(kSep);
      iov[4].iov_len = kSepLen;
      iov[5].iov_base = const_cast<char*>(content);
      iov[5].iov_len = content_len;
      lock();
      [[maybe_unused]] ssize_t written = writev(kStdOutFd, iov, 6);
      unlock();
    } else {
      struct iovec iov[3];
      iov[0].iov_base = const_cast<char*>(level_str);
      iov[0].iov_len = level_len;
      iov[1].iov_base = const_cast<char*>(kSep);
      iov[1].iov_len = kSepLen;
      iov[2].iov_base = const_cast<char*>(content);
      iov[2].iov_len = content_len;
      lock();
      [[maybe_unused]] ssize_t written = writev(kStdOutFd, iov, 3);
      unlock();
    }
#endif
  }

  inline static std::mutex& stdout_mutex() {
    static std::mutex stdout_mutex;
    return stdout_mutex;
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
    return mode_ == ColorMode::kAlways ||
           (mode_ == ColorMode::kAuto && is_ansi_sequence_available());
  }

  std::unique_ptr<char[]> buffer_;
  std::size_t cursor_ = 0;
  ColorMode mode_ = ColorMode::kAuto;

  static constexpr int kStdOutFd = 1;
  static constexpr std::size_t kBufferCapacity = 4096;

  static constexpr const char* kSep = ": ";
  static constexpr const char* kBold = core::kBold;
  static constexpr const char* kReset = core::kReset;
  static constexpr const std::size_t kSepLen = 2;
  static constexpr const std::size_t kBoldLen = 4;
  static constexpr const std::size_t kResetLen = 4;
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_SINKS_STDOUT_SINK_H_
