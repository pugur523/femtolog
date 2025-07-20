// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_SINKS_JSON_LINES_SINK_H_
#define INCLUDE_FEMTOLOG_SINKS_JSON_LINES_SINK_H_

#include <fcntl.h>

#include <memory>
#include <string>

#include "femtolog/base/log_entry.h"
#include "femtolog/base/log_level.h"
#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/file_util.h"
#include "femtolog/sinks/sink_base.h"

#if FEMTOLOG_IS_WINDOWS
#include <io.h>
#include <windows.h>
#else
#include <sys/uio.h>
#include <unistd.h>
#endif

namespace femtolog {

template <bool enable_buffering = true>
class JsonLinesSink final : public SinkBase {
 public:
  explicit JsonLinesSink(const std::string& file_path) : file_path_(file_path) {
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

    if constexpr (enable_buffering) {
      buffer_ = std::make_unique<char[]>(kBufferCapacity);
    }
  }

  JsonLinesSink()
      : JsonLinesSink(
            core::join_path(core::exe_dir(), "logs", "jsonl", "latest.jsonl")) {
  }

  ~JsonLinesSink() override {
    if constexpr (enable_buffering) {
      flush();
    }

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
    // Pre-format JSON line
    char line[kMaxJsonLineSize];
    const char* level_str = log_level_to_lower_str(entry.level);
    std::string encoded = core::encode_escape(content, len);
    auto result = fmt::format_to_n(
        line, kMaxJsonLineSize,
        R"({{"timestamp": {}, "level": "{}", "message": "{}"}})"
        "\n",
        entry.timestamp_ns, level_str, fmt::string_view(encoded));
    std::size_t line_len = result.size;

    if constexpr (!enable_buffering) {
      write_raw(line, line_len);
      return;
    }

    if (cursor_ + line_len > kBufferCapacity) {
      flush();
    }

    if (line_len > kBufferCapacity) {
      write_raw(line, line_len);
      return;
    }

    std::memcpy(buffer_.get() + cursor_, line, line_len);
    cursor_ += line_len;
  }

 private:
  inline void flush() {
    if constexpr (enable_buffering) {
      if (cursor_ == 0 || !buffer_ || fd_ < 0) {
        return;
      }
#if FEMTOLOG_IS_WINDOWS
      _write(fd_, buffer_.get(), static_cast<uint32_t>(cursor_));
#else
      [[maybe_unused]] ssize_t written = write(fd_, buffer_.get(), cursor_);
#endif
      cursor_ = 0;
    }
  }

  inline void write_raw(const char* data, std::size_t size) {
    if (fd_ < 0) {
      return;
    }

#if FEMTOLOG_IS_WINDOWS
    _write(fd_, data, static_cast<uint32_t>(size));
#else
    [[maybe_unused]] ssize_t written = write(fd_, data, size);
#endif
  }

  int fd_ = -1;

  std::string file_path_;
  std::unique_ptr<char[]> buffer_;
  std::size_t cursor_ = 0;

  static constexpr std::size_t kBufferCapacity = 8192;
  static constexpr std::size_t kMaxJsonLineSize = 2048;
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_SINKS_JSON_LINES_SINK_H_
