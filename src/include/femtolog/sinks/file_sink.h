// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_SINKS_FILE_SINK_H_
#define INCLUDE_FEMTOLOG_SINKS_FILE_SINK_H_

#include <fcntl.h>

#include <cstring>
#include <memory>
#include <string>

#include "femtolog/base/log_entry.h"
#include "femtolog/base/log_level.h"
#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/file_util.h"
#include "femtolog/sinks/sink_base.h"
#include "fmt/chrono.h"
#include "fmt/format.h"

#if FEMTOLOG_IS_WINDOWS
#include <io.h>
#include <windows.h>
#else
#include <sys/uio.h>
#include <unistd.h>
#endif

namespace femtolog {

class FileSink final : public SinkBase {
 public:
  explicit FileSink(const std::string& file_path) : file_path_(file_path) {
    std::string parent_dir = core::parent_dir(file_path_);
    if (!core::dir_exists(parent_dir.c_str())) {
      core::create_directories(parent_dir.c_str());
    }

    if (core::file_exists(file_path_.c_str())) {
      constexpr std::size_t kTimestampBufSize = 32;
      char timestamp_buf[kTimestampBufSize];
      std::size_t timestamp_size =
          format_timestamp<TimeZone::kLocal, "{:%Y-%m-%d_%H-%M-%S}">(
              timestamp_ns(), timestamp_buf, kTimestampBufSize);
      std::string base_timestamp_name(timestamp_buf, timestamp_size);

      std::string original_file_name_without_ext =
          core::file_name_without_extension(file_path_);
      std::string original_extension = core::file_extension(file_path_);

      int counter = 0;
      std::string compressed_file_name;
      std::string dest_path;

      do {
        compressed_file_name = original_file_name_without_ext;
        compressed_file_name.append("_");
        compressed_file_name.append(base_timestamp_name);

        if (counter > 0) {
          compressed_file_name.append("-");
          compressed_file_name.append(std::to_string(counter));
        }

        compressed_file_name.append(".");
        compressed_file_name.append(original_extension);
        compressed_file_name.append(".gz");

        dest_path = core::join_path(parent_dir, compressed_file_name);
        counter++;
      } while (core::file_exists(dest_path.c_str()));

      core::compress(file_path_.c_str(), dest_path.c_str(), true);
    }
    core::create_file(file_path_.c_str());

#if FEMTOLOG_IS_WINDOWS
    fd_ =
        _open(file_path_.c_str(), _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY,
              _S_IREAD | _S_IWRITE);
#else
    fd_ = open(file_path_.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
#endif

    buffer_ = std::make_unique<char[]>(kBufferCapacity);
  }

  FileSink()
      : FileSink(core::join_path(core::exe_dir(), "logs", "latest.log")) {}

  ~FileSink() override {
    flush();

    if (fd_ >= 0) {
#if FEMTOLOG_IS_WINDOWS
      _close(fd_);
#else
      close(fd_);
#endif
    }
  }

  inline void on_log(const LogEntry& entry,
                     const char* content,
                     std::size_t len) override {
    constexpr std::size_t kTimestampBufSize = 32;
    char timestamp_buf[kTimestampBufSize];
    const std::size_t timestamp_size =
        format_timestamp<TimeZone::kLocal, "[{:%H:%M:%S}.{:09d}] ">(
            entry.timestamp_ns, timestamp_buf, kTimestampBufSize);

    const char* level_str = log_level_to_lower_str(entry.level);
    const std::size_t lv_len = level_len(entry.level);

    const std::size_t total = timestamp_size + lv_len + kSepLen + len;

    if (cursor_ + total > kBufferCapacity) {
      flush();
    }

    if (total > kBufferCapacity) {
      write_direct(timestamp_buf, timestamp_size, level_str, lv_len, content,
                   len);
      return;
    }

    char* buffer = buffer_.get();
    std::memcpy(buffer + cursor_, timestamp_buf, timestamp_size);
    cursor_ += timestamp_size;
    std::memcpy(buffer + cursor_, level_str, lv_len);
    cursor_ += lv_len;
    std::memcpy(buffer + cursor_, kSep, kSepLen);
    cursor_ += kSepLen;
    std::memcpy(buffer + cursor_, content, len);
    cursor_ += len;
  }

 private:
  inline void flush() {
    if (cursor_ == 0 || !buffer_ || fd_ < 0) {
      return;
    }
#if FEMTOLOG_IS_WINDOWS
    _write(fd_, buffer_.get(), static_cast<unsigned int>(cursor_));
#else
    const auto _ = write(fd_, buffer_.get(), cursor_);
#endif
    cursor_ = 0;
  }

  inline void write_direct(const char* timestamp_buf,
                           std::size_t timestamp_size,
                           const char* level_str,
                           std::size_t level_len,
                           const char* content,
                           std::size_t content_len) {
    if (fd_ < 0) {
      return;
    }

#if FEMTOLOG_IS_WINDOWS
    constexpr const std::size_t kTmpBufSize = 4096;
    std::size_t total = timestamp_size + level_len + kSepLen + content_len;
    if (total <= kTmpBufSize) {
      char tmp[kTmpBufSize];
      char* p = tmp;
      std::memcpy(p, timestamp_buf, timestamp_size);
      p += timestamp_size;
      std::memcpy(p, level_str, level_len);
      p += level_len;
      std::memcpy(p, kSep, kSepLen);
      p += kSepLen;
      std::memcpy(p, content, content_len);
      _write(fd_, tmp, static_cast<unsigned int>(total));
    } else {
      _write(fd_, timestamp_buf, static_cast<unsigned int>(timestamp_size));
      _write(fd_, level_str, static_cast<unsigned int>(level_len));
      _write(fd_, kSep, static_cast<unsigned int>(kSepLen));
      _write(fd_, content, static_cast<unsigned int>(content_len));
    }
#else
    struct iovec iov[3];
    int iovcnt = 0;
    iov[iovcnt++] = {const_cast<char*>(timestamp_buf), timestamp_size};
    iov[iovcnt++] = {const_cast<char*>(level_str), level_len};
    iov[iovcnt++] = {const_cast<char*>(kSep), kSepLen};
    iov[iovcnt++] = {const_cast<char*>(content), content_len};
    const auto _ = writev(fd_, iov, iovcnt);
#endif
  }

  int fd_ = -1;

  std::string file_path_;
  std::unique_ptr<char[]> buffer_;
  std::size_t cursor_ = 0;

  static constexpr std::size_t kBufferCapacity = 4096;

  static constexpr const char* kSep = ": ";
  static constexpr std::size_t kSepLen = 2;
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_SINKS_FILE_SINK_H_
