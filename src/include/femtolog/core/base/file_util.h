// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef CORE_BASE_FILE_UTIL_H_
#define CORE_BASE_FILE_UTIL_H_

#include <sys/stat.h>

#include <cstddef>
#include <string>
#include <vector>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/core_export.h"

#if FEMTOLOG_ENABLE_AVX2
#include <immintrin.h>
#endif  // FEMTOLOG_ENABLE_AVX2

namespace core {

constexpr const std::size_t kPathMaxLength = 4096;
constexpr const std::size_t kPredictedFilesNbPerDir = 64;
using Files = std::vector<std::string>;

[[nodiscard]] FEMTOLOG_CORE_EXPORT bool file_exists(const char* file_name);
[[nodiscard]] FEMTOLOG_CORE_EXPORT bool dir_exists(const char* dir_name);
[[nodiscard]] FEMTOLOG_CORE_EXPORT std::string read_file(const char* path);
[[nodiscard]] FEMTOLOG_CORE_EXPORT const std::string& exe_path();
[[nodiscard]] FEMTOLOG_CORE_EXPORT const std::string& exe_dir();
[[nodiscard]] FEMTOLOG_CORE_EXPORT const std::string& resources_dir();
[[nodiscard]] FEMTOLOG_CORE_EXPORT bool is_executable_in_path(const char* path);
[[nodiscard]] FEMTOLOG_CORE_EXPORT Files list_files(const std::string& path);
[[nodiscard]] FEMTOLOG_CORE_EXPORT std::string parent_dir(
    const std::string& path);
[[nodiscard]] FEMTOLOG_CORE_EXPORT std::string base_name(
    const std::string& path);
[[nodiscard]] FEMTOLOG_CORE_EXPORT std::string temp_directory();
[[nodiscard]] FEMTOLOG_CORE_EXPORT std::string temp_path(
    const std::string& prefix);
FEMTOLOG_CORE_EXPORT bool compress(const char* src_path,
                                   const char* dest_path,
                                   bool remove_after_compress = true);

FEMTOLOG_CORE_EXPORT int create_directory(const char* path);
FEMTOLOG_CORE_EXPORT int create_directories(const char* path);
FEMTOLOG_CORE_EXPORT int create_file(const char* path);
FEMTOLOG_CORE_EXPORT int remove_file(const char* path);
FEMTOLOG_CORE_EXPORT int remove_directory(const char* path);
FEMTOLOG_CORE_EXPORT int rename_file(const char* old_path,
                                     const char* new_path);
FEMTOLOG_CORE_EXPORT int write_file(const char* path,
                                    const std::string& content);

FEMTOLOG_CORE_EXPORT int write_binary_to_file(const void* binary_data,
                                              std::size_t binary_size,
                                              const std::string& output_path);

[[nodiscard]] FEMTOLOG_CORE_EXPORT std::string file_extension(
    const std::string& path);

[[nodiscard]] FEMTOLOG_CORE_EXPORT std::string sanitize_component(
    const char* part,
    bool is_first);

template <typename T>
inline int write_binary_to_file(const std::vector<T>& data,
                                const std::string& output_path) {
  return write_binary_to_file(static_cast<const void*>(data.data()),
                              data.size() * sizeof(T), output_path);
}

template <typename T>
[[nodiscard]] inline std::string to_string_path_part(const T& part) {
  if constexpr (std::is_convertible_v<T, std::string>) {
    return static_cast<std::string>(part);
  } else if constexpr (std::is_convertible_v<T, const char*>) {
    return std::string(part);
  } else {
    static_assert(std::is_same_v<T, void>,
                  "to_string_path_part: unsupported path element type");
  }
}

template <typename... Parts>
[[nodiscard]] std::string join_path(const Parts&... parts) {
  std::string joined;
  joined.reserve(kPathMaxLength);
  bool is_first = true;

  (
      [&] {
        std::string part = to_string_path_part(parts);
        std::string sanitized = sanitize_component(part.c_str(), is_first);
        if (!sanitized.empty()) {
          if (!is_first) {
            joined.push_back(FEMTOLOG_DIR_SEPARATOR);
          }
          joined.append(sanitized);
          is_first = false;
        }
      }(),
      ...);

  return joined;
}

[[nodiscard]] FEMTOLOG_CORE_EXPORT std::vector<std::string> read_lines_default(
    const std::string& content);

#if FEMTOLOG_ENABLE_AVX2
[[nodiscard]] FEMTOLOG_CORE_EXPORT std::vector<std::string>
read_lines_with_avx2(const std::string& content);
#endif

template <bool use_avx2_if_available = true>
inline std::vector<std::string> read_lines(const std::string& content) {
#if FEMTOLOG_ENABLE_AVX2
  if constexpr (use_avx2_if_available) {
    return read_lines_with_avx2(content);
  }
#endif
  return read_lines_default(content);
}

class FEMTOLOG_CORE_EXPORT TempFile {
 public:
  explicit TempFile(const std::string& prefix = "tmp_",
                    const std::string& content = "");

  ~TempFile();

  inline constexpr const std::string& path() const { return path_; }
  inline constexpr bool valid() const { return valid_; }

 private:
  std::string path_;
  bool valid_ : 1 = true;
};

class FEMTOLOG_CORE_EXPORT TempDir {
 public:
  explicit TempDir(const std::string& prefix = "tmp_dir_");

  ~TempDir();

  inline constexpr const std::string& path() const { return path_; }
  inline constexpr bool valid() const { return valid_; }

 private:
  std::string path_;
  bool valid_ : 1 = true;
};

class FEMTOLOG_CORE_EXPORT File {
 public:
  File(std::string&& file_name, std::string&& source);
  explicit File(std::string&& file_name);

  ~File() = default;

  File(const File&) = delete;
  File& operator=(const File&) = delete;

  File(File&&) noexcept = default;
  File& operator=(File&&) noexcept = default;

  inline constexpr const std::string& file_name() const { return file_name_; }
  inline constexpr const std::string& source() const { return source_; }
  inline const std::string& line(std::size_t line_no) const {
#if FEMTOLOG_IS_DEBUG
    static const std::string empty = "";
    if (line_no == 0 || line_no > lines_.size()) {
      return empty;
    }
#endif
    return lines_[line_no - 1];
  }

 private:
  std::string file_name_ = "";
  std::string source_ = "";
  std::vector<std::string> lines_;
};

}  // namespace core

#endif  // CORE_BASE_FILE_UTIL_H_
