// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/core/base/file_util.h"

#include <limits.h>
#include <zlib.h>

#include <chrono>
#include <utility>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/check.h"

#if FEMTOLOG_IS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#undef APIENTRY
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace femtolog::core {

bool file_exists(const char* file_name) {
#if FEMTOLOG_IS_WINDOWS
  DWORD attributes = GetFileAttributesA(file_name);
  return (attributes != INVALID_FILE_ATTRIBUTES &&
          !(attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat buffer;
  return (stat(file_name, &buffer) == 0) && S_ISREG(buffer.st_mode);
#endif
}

bool dir_exists(const char* dir_name) {
#if FEMTOLOG_IS_WINDOWS
  DWORD attributes = GetFileAttributesA(dir_name);
  return (attributes != INVALID_FILE_ATTRIBUTES &&
          (attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat buffer;
  return stat(dir_name, &buffer) == 0 && S_ISDIR(buffer.st_mode);
#endif
}

std::string read_file(const char* path) {
#if FEMTOLOG_IS_WINDOWS
  int fd = _open(path, _O_RDONLY | _O_BINARY);
#else
  int fd = open(path, O_RDONLY);
#endif
  if (fd < 0) {
    std::cerr << "Failed to open file: " << path << " (" << std::strerror(errno)
              << ")\n";
    return "";
  }

#if FEMTOLOG_IS_WINDOWS
  struct _stat64i32 st;
  if (_fstat(fd, &st) != 0)
#else
  struct stat st;
  if (fstat(fd, &st) != 0)
#endif
  {
    std::cerr << "Failed to stat file: " << path << " (" << std::strerror(errno)
              << ")\n";
#if FEMTOLOG_IS_WINDOWS
    _close(fd);
#else
    close(fd);
#endif
    return "";
  }

  std::string contents;
  if (st.st_size > 0) {
    contents.resize(static_cast<std::size_t>(st.st_size));
    std::size_t total_read = 0;
    while (total_read < static_cast<std::size_t>(st.st_size)) {
#if FEMTOLOG_IS_WINDOWS
      int bytes = _read(fd, &contents[total_read],
                        static_cast<unsigned int>(st.st_size - total_read));
#else
      ssize_t bytes = read(fd, &contents[total_read], st.st_size - total_read);
#endif
      if (bytes <= 0) {
        break;
      }
      total_read += static_cast<std::size_t>(bytes);
    }
    contents.resize(total_read);
  }

#if FEMTOLOG_IS_WINDOWS
  _close(fd);
#else
  close(fd);
#endif
  return contents;
}

const std::string& exe_path() {
  static const std::string cached_path = []() -> std::string {
#if FEMTOLOG_IS_WINDOWS
    char path[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(nullptr, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
      return "";
    }
    return (len == 0 || len >= MAX_PATH) ? "" : std::string(path, len);
#else
    char path[kPathMaxLength] = {};
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    return len == -1 ? "" : std::string(path, len);
#endif
  }();

  return cached_path;
}

const std::string& exe_dir() {
  static const std::string cached_dir = []() -> std::string {
    const std::string& this_exe_path = exe_path();
    if (this_exe_path.empty()) {
      return "";
    }

    return parent_dir(this_exe_path);
  }();
  return cached_dir;
}

const std::string& resources_dir() {
  static const std::string cached_resources_dir =
      join_path(exe_dir(), "resources");
  return cached_resources_dir;
}

bool is_executable_in_path(const char* path) {
#if FEMTOLOG_IS_WINDOWS
  const char* pathext = std::getenv("PATHEXT");
  std::vector<std::string> exts;
  if (pathext) {
    std::string extstr = pathext;
    std::size_t start = 0, end;
    while ((end = extstr.find(';', start)) != std::string::npos) {
      exts.push_back(extstr.substr(start, end - start));
      start = end + 1;
    }
    exts.push_back(extstr.substr(start));
  } else {
    exts = {".EXE", ".BAT", ".CMD"};
  }
#endif

  const char* path_env = std::getenv("PATH");
  if (!path_env) {
    return false;
  }
  std::string path_var = path_env;
  std::size_t start = 0, end;
  while ((end = path_var.find(FEMTOLOG_PATH_SEPARATOR, start)) !=
         std::string::npos) {
    std::string dir = path_var.substr(start, end - start);
    std::string full_path = dir + FEMTOLOG_DIR_SEPARATOR + path;
#if FEMTOLOG_IS_WINDOWS
    for (const auto& ext : exts) {
      std::string candidate = full_path + ext;
      if (file_exists(candidate.c_str())) {
        return true;
      }
    }
#else
    if (file_exists(full_path.c_str()) &&
        access(full_path.c_str(), X_OK) == 0) {
      return true;
    }
#endif
    start = end + 1;
  }
  // Check the last path entry
  std::string dir = path_var.substr(start);
  std::string full_path = dir + FEMTOLOG_DIR_SEPARATOR + path;
#if FEMTOLOG_IS_WINDOWS
  for (const auto& ext : exts) {
    std::string candidate = full_path + ext;
    if (file_exists(candidate.c_str())) {
      return true;
    }
  }
#else
  if (file_exists(full_path.c_str()) && access(full_path.c_str(), X_OK) == 0) {
    return true;
  }
#endif
  return false;
}

Files list_files(const std::string& path) {
  Files files;
  files.reserve(kPredictedFilesNbPerDir);

#if FEMTOLOG_IS_WINDOWS
  std::string search_path = path + "\\*";
  WIN32_FIND_DATAA find_data;
  HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);

  if (hFind == INVALID_HANDLE_VALUE) {
    std::cerr << "Cannot open directory: " << path << "\n";
    return files;
  }

  do {
    std::string name = find_data.cFileName;
    if (name != "." && name != "..") {
      files.emplace_back(name);
    }
  } while (FindNextFileA(hFind, &find_data) != 0);

  FindClose(hFind);
  return files;

#else
  DIR* dir = opendir(path.c_str());
  if (!dir) {
    std::cerr << "Cannot open directory: " << path << "\n";
    return files;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string name = entry->d_name;
    if (name != "." && name != "..") {
      files.emplace_back(name);
    }
  }

  closedir(dir);
#endif
  return files;
}

std::string parent_dir(const std::string& path) {
  std::size_t pos = path.find_last_of(FEMTOLOG_DIR_SEPARATOR);
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(0, pos);
}

std::string base_name(const std::string& path) {
  std::size_t pos = path.find_last_of(FEMTOLOG_DIR_SEPARATOR);
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(pos + 1);
}

std::string temp_directory() {
#if FEMTOLOG_IS_WINDOWS
  char buffer[MAX_PATH];
  DWORD len = GetTempPathA(MAX_PATH, buffer);
  if (len == 0 || len > MAX_PATH) {
    return ".\\";
  }
  return std::string(buffer);
#else
  const char* env_vars[] = {"TMPDIR", "TMP", "TEMP", "TEMPDIR"};
  for (const char* var : env_vars) {
    const char* value = std::getenv(var);
    if (value && *value) {
      return std::string(value) + FEMTOLOG_DIR_SEPARATOR;
    }
  }
  return "/tmp/";
#endif
}

std::string temp_path(const std::string& prefix) {
  std::string dir = temp_directory();

  static constexpr char charset[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  constexpr std::size_t length = 8;

  std::mt19937 rng(static_cast<unsigned>(
      std::chrono::steady_clock::now().time_since_epoch().count()));
  std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

  std::string suffix = "femtolog_";
  for (std::size_t i = 0; i < length; ++i) {
    suffix += charset[dist(rng)];
  }

#if FEMTOLOG_IS_WINDOWS
  return dir + prefix + suffix + ".tmp";
#else
  return dir + prefix + suffix;
#endif
}

bool compress(const char* src_path,
              const char* dest_path,
              bool remove_after_compress) {
  FEMTOLOG_DCHECK(file_exists(src_path)) << src_path << " not exists";
  FEMTOLOG_DCHECK(!file_exists(dest_path)) << dest_path << " exists";
  FILE* src = std::fopen(src_path, "rb");
  if (!src) {
    return false;
  }

  gzFile dst = gzopen(dest_path, "wb");
  if (!dst) {
    std::fclose(src);
    return false;
  }

  constexpr std::size_t kBufSize = 8192;
  char buf[kBufSize];
  while (std::size_t read = std::fread(buf, 1, kBufSize, src)) {
    if (gzwrite(dst, buf, static_cast<unsigned int>(read)) == 0) {
      gzclose(dst);
      std::fclose(src);
      return false;
    }
  }

  gzclose(dst);
  std::fclose(src);

  if (remove_after_compress) {
    std::remove(src_path);
  }
  return true;
}

int create_directory(const char* path) {
#if FEMTOLOG_IS_WINDOWS
  BOOL ok = CreateDirectoryA(path, nullptr);
  if (!ok && GetLastError() != ERROR_ALREADY_EXISTS) {
    return -1;
  }
  return 0;
#else
  return mkdir(path, 0755) == 0 || errno == EEXIST ? 0 : -1;
#endif
}

int create_directories(const char* path) {
  std::string sanitized_path = sanitize_component(path, true);
  std::size_t pos = 0;
  int result = 0;

  while (pos < sanitized_path.size()) {
    std::size_t next_pos = sanitized_path.find(FEMTOLOG_DIR_SEPARATOR, pos);
    std::string dir;

    if (next_pos != std::string::npos) {
      dir = sanitized_path.substr(0, next_pos);
      pos = next_pos + 1;
    } else {
      dir = sanitized_path;
      pos = sanitized_path.size();
    }

    if (!dir.empty() && !dir_exists(dir.c_str())) {
      if (create_directory(dir.c_str()) != 0) {
        std::cerr << "Failed to create directory: " << dir << "\n";
        result++;
      }
    }
  }

  return result;
}

int create_file(const char* path) {
#if FEMTOLOG_IS_WINDOWS
  FILE* fp = nullptr;
  fopen_s(&fp, path, "wx");  // "w"=write, "x"=fail if exists
  if (!fp) {
    return -1;
  }
#else
  FILE* fp = fopen(path, "wx");  // POSIX: 'x' = exclusive (fail if exists)
  if (!fp) {
    return -1;
  }
#endif
  fclose(fp);
  return 0;
}

int remove_file(const char* path) {
#if FEMTOLOG_IS_WINDOWS
  return DeleteFileA(path) ? 0 : -1;
#else
  return std::remove(path);
#endif
}

int remove_directory(const char* path) {
#if FEMTOLOG_IS_WINDOWS
  return RemoveDirectoryA(path) ? 0 : -1;
#else
  return rmdir(path);
#endif
}

int rename_file(const char* old_path, const char* new_path) {
#if FEMTOLOG_IS_WINDOWS
  return MoveFileA(old_path, new_path) ? 0 : -1;
#else
  return std::rename(old_path, new_path);
#endif
}

int write_file(const char* path, const std::string& content) {
#if FEMTOLOG_IS_WINDOWS
  int fd = _open(path, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY,
                 _S_IREAD | _S_IWRITE);
#else
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
#endif

  if (fd < 0) {
    std::cerr << "Failed to open file for writing: " << path << " ("
              << std::strerror(errno) << ")\n";
    return -1;
  }

  std::size_t total_written = 0;
  const char* data = content.data();
  std::size_t size = content.size();

  while (total_written < size) {
#if FEMTOLOG_IS_WINDOWS
    int bytes = _write(fd, data + total_written,
                       static_cast<unsigned int>(size - total_written));
#else
    ssize_t bytes = write(fd, data + total_written, size - total_written);
#endif
    if (bytes <= 0) {
      std::cerr << "Failed to write to file: " << path << " ("
                << std::strerror(errno) << ")\n";
#if FEMTOLOG_IS_WINDOWS
      _close(fd);
#else
      close(fd);
#endif
      return -1;
    }
    total_written += static_cast<std::size_t>(bytes);
  }

#if FEMTOLOG_IS_WINDOWS
  _close(fd);
#else
  close(fd);
#endif
  return 0;
}

int write_binary_to_file(const void* binary_data,
                         std::size_t binary_size,
                         const std::string& output_path) {
  if (!binary_data || binary_size == 0) {
    std::cerr << "Invalid data or size (null or zero) for file: " << output_path
              << "\n";
    return 4;
  }

  FILE* fp = fopen(output_path.c_str(), "wb");
  if (!fp) {
    std::cerr << "Failed to open file for writing: " << output_path << "\n";
    std::perror("fopen");
    return 2;
  }

  std::size_t written = fwrite(binary_data, 1, binary_size, fp);
  fflush(fp);
  fclose(fp);

  if (written != binary_size) {
    std::cerr << "Failed to write all data to file: " << output_path
              << " (written " << written << " / " << binary_size << ")"
              << "\n";
    std::perror("fwrite");
    return 1;
  }

  return 0;
}

std::string file_extension(const std::string& path) {
  std::size_t last_slash = path.find_last_of("/\\");
  std::size_t last_dot = path.find_last_of('.');

  if (last_dot == std::string::npos ||
      (last_slash != std::string::npos && last_dot < last_slash)) {
    return "";  // No extension or dot is part of directory
  }

  // Handle hidden files like ".gitignore"
  if (last_dot == 0 ||
      (last_slash != std::string::npos && last_dot == last_slash + 1)) {
    return "";
  }

  return path.substr(last_dot + 1);
}

std::string sanitize_component(const char* part, bool is_first) {
  std::string result = part;

  // Remove leading separator if not first
  if (!is_first && !result.empty() &&
      result.front() == FEMTOLOG_DIR_SEPARATOR) {
    result.erase(0, 1);
  }

  // Remove trailing separator
  if (!result.empty() && result.back() == FEMTOLOG_DIR_SEPARATOR) {
    result.pop_back();
  }

  return result;
}

std::vector<std::string> read_lines_default(const std::string& content) {
  std::vector<std::string> lines;

  if (content.empty()) {
    return lines;
  }

  std::size_t estimated_lines =
      std::count(content.begin(), content.end(), '\n') + 1;
  lines.reserve(estimated_lines);

  const char* current_pos = content.data();
  const char* end_pos = current_pos + content.size();

  while (current_pos < end_pos) {
    const char* next_newline = static_cast<const char*>(
        memchr(current_pos, '\n', end_pos - current_pos));

    const char* line_end = next_newline ? next_newline : end_pos;
    std::size_t line_length = line_end - current_pos;

    // CRLF
    if (line_length > 0 && line_end[-1] == '\r') {
      --line_length;
    }

    lines.emplace_back(current_pos, line_length);
    current_pos = next_newline ? next_newline + 1 : end_pos;
  }

  return lines;
}

#if FEMTOLOG_ENABLE_AVX2

std::vector<std::string> read_lines_with_avx2(const std::string& content) {
  std::vector<std::string> lines;

  if (content.empty()) {
    return lines;
  }

  // predicts as 80 chars per line.
  std::vector<std::size_t> newline_positions;
  newline_positions.reserve(content.size() / 80);

  const char* data = content.data();
  std::size_t size = content.size();
  std::size_t pos = 0;

  // search line breaks with AVX2 (processes 32 bytes at a time)
  if (size >= 32) {
    const __m256i newline_vec = _mm256_set1_epi8('\n');

    for (; pos + 32 <= size; pos += 32) {
      __m256i chunk =
          _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data + pos));
      __m256i cmp = _mm256_cmpeq_epi8(chunk, newline_vec);
      uint32_t mask = _mm256_movemask_epi8(cmp);

      while (mask) {
        int offset = __builtin_ctz(mask);
        newline_positions.push_back(pos + offset);
        mask &= mask - 1;
      }
    }
  }

  // scalar processing for remaining
  for (; pos < size; ++pos) {
    if (data[pos] == '\n') {
      newline_positions.push_back(pos);
    }
  }

  lines.reserve(newline_positions.size() + 1);

  std::size_t line_start = 0;
  for (std::size_t newline_pos : newline_positions) {
    std::size_t line_length = newline_pos - line_start;

    // CRLF
    if (line_length > 0 && data[line_start + line_length - 1] == '\r') {
      --line_length;
    }

    lines.emplace_back(data + line_start, line_length);
    line_start = newline_pos + 1;
  }

  // last line
  if (line_start < size) {
    std::size_t line_length = size - line_start;
    if (line_length > 0 && data[line_start + line_length - 1] == '\r') {
      --line_length;
    }
    lines.emplace_back(data + line_start, line_length);
  }

  return lines;
}

#endif  // FEMTOLOG_ENABLE_AVX2

TempFile::TempFile(const std::string& prefix, const std::string& content) {
  path_ = temp_path(prefix);
  if (create_file(path_.c_str()) != 0) {
    std::cerr << "Failed to create temp file: " << path_ << "\n";
    valid_ = false;
    return;
  }
  if (!content.empty()) {
    if (write_file(path_.c_str(), content) != 0) {
      std::cerr << "Failed to write to temp file: " << path_ << "\n";
      valid_ = false;
    }
  }
}

TempFile::~TempFile() {
  if (valid_) {
    if (remove_file(path_.c_str()) != 0) {
      std::cerr << "Failed to remove temp file: " << path_ << "\n";
    }
  }
}

TempDir::TempDir(const std::string& prefix) {
  path_ = temp_path(prefix);
  if (create_directory(path_.c_str()) != 0) {
    std::cerr << "Failed to create temp directory: " << path_ << "\n";
    valid_ = false;
  }
}

TempDir::~TempDir() {
  if (valid_) {
    if (remove_directory(path_.c_str()) != 0) {
      std::cerr << "Failed to remove temp directory: " << path_ << "\n";
    }
  }
}

File::File(std::string&& file_name, std::string&& source)
    : file_name_(std::move(file_name)),
      source_(std::move(source)),
      lines_(read_lines<true>(source_)) {}

File::File(std::string&& file_name)
    : File(std::move(file_name), read_file(file_name.c_str())) {}

}  // namespace femtolog::core
