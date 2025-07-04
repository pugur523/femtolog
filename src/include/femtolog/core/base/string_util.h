// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_CORE_BASE_STRING_UTIL_H_
#define INCLUDE_FEMTOLOG_CORE_BASE_STRING_UTIL_H_

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <format>
#include <queue>
#include <string>
#include <string_view>
#include <utility>

#include "femtolog/core/base/core_export.h"

namespace core {

[[nodiscard]] CORE_EXPORT std::string encode_escape(std::string_view input);
[[nodiscard]] CORE_EXPORT std::string encode_escape(const char* s,
                                                    std::size_t len);
[[nodiscard]] CORE_EXPORT std::string decode_escape(std::string_view input);
[[nodiscard]] CORE_EXPORT std::string decode_escape(const char* s,
                                                    std::size_t len);

[[nodiscard]] constexpr char to_lower(char c) {
  return (c >= 'A' && c <= 'Z') ? (c | 0x20) : c;
}
CORE_EXPORT void to_lower(char* input, std::size_t len);
CORE_EXPORT void to_lower(char* input);
CORE_EXPORT void to_lower(std::string* input);
[[nodiscard]] CORE_EXPORT std::string to_lower(const std::string& input);

[[nodiscard]] constexpr char to_upper(char c) {
  return (c >= 'a' && c <= 'z') ? (c & ~0x20) : c;
}
CORE_EXPORT void to_upper(char* input, std::size_t len);
CORE_EXPORT void to_upper(char* input);
CORE_EXPORT void to_upper(std::string* input);
[[nodiscard]] CORE_EXPORT std::string to_upper(const std::string& input);

[[nodiscard]] CORE_EXPORT std::size_t utf8_char_length(unsigned char lead);
[[nodiscard]] CORE_EXPORT std::string utf8_truncate(const std::string& input,
                                                    std::size_t max_chars);
[[nodiscard]] CORE_EXPORT std::queue<std::string> split_string(
    const std::string& input,
    const std::string& delimiter);
[[nodiscard]] CORE_EXPORT std::string remove_bracket(
    const std::string& input,
    std::size_t max_nest_size = 32);
[[nodiscard]] CORE_EXPORT std::size_t safe_strlen(const char* str);

CORE_EXPORT void format_address_safe(uintptr_t addr,
                                     char* buffer_start,
                                     std::size_t buffer_size);

CORE_EXPORT void padding(char*& cursor,
                         const char* const end,
                         std::size_t current_len,
                         std::size_t align_len);

CORE_EXPORT std::size_t write_raw(char*& dest,
                                  const char* source,
                                  std::size_t len);

template <typename... Args>
std::size_t write_format(char*& cursor,
                         const char* const end,
                         std::format_string<Args...> fmt,
                         Args&&... args) {
  std::ptrdiff_t remaining = end - cursor;
  if (remaining <= 0) {
    return 0;
  }

  auto result =
      std::format_to_n(cursor, static_cast<std::size_t>(remaining - 1), fmt,
                       std::forward<Args>(args)...);
  std::size_t written = result.out - cursor;

  cursor += written;
  *cursor = '\0';
  return written;
}

inline void write_bytes(char*& cursor, const void* data, std::size_t size) {
  std::memcpy(cursor, data, size);
  cursor += size;
}

inline void read_bytes(const char*& cursor, void* data, std::size_t size) {
  std::memcpy(data, cursor, size);
  cursor += size;
}

inline bool starts_with(const std::string& input,
                        const std::string& prefix,
                        std::size_t index = 0) {
  return input.size() >= prefix.size() &&
         input.compare(index, prefix.size(), prefix) == 0;
}

template <typename T>
struct is_ostreamable {
 private:
  template <typename U>
  static auto test(int)  // NOLINT
      -> decltype(std::declval<std::ostream&>() << std::declval<U>(),
                  std::true_type{});

  template <typename>
  static std::false_type test(...);

 public:
  static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
inline constexpr bool is_ostreamable_v = is_ostreamable<T>::value;

constexpr const char* kReset = "\033[0m";
constexpr const char* kBold = "\033[1m";
constexpr const char* kRed = "\033[31m";
constexpr const char* kYellow = "\033[33m";
constexpr const char* kGreen = "\033[32m";
constexpr const char* kCyan = "\033[36m";
constexpr const char* kMagenta = "\033[35m";
constexpr const char* kGray = "\033[90m";
constexpr const char* kBright_red = "\033[91m";
constexpr const char* kBright_green = "\033[92m";
constexpr const char* kBright_cyan = "\033[96m";

}  // namespace core

#endif  // INCLUDE_FEMTOLOG_CORE_BASE_STRING_UTIL_H_
