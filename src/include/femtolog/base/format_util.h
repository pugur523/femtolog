// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BASE_FORMAT_UTIL_H_
#define INCLUDE_FEMTOLOG_BASE_FORMAT_UTIL_H_

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "femtolog/build/build_flag.h"
#include "fmt/args.h"
#include "fmt/core.h"
#include "fmt/format.h"

#if FEMTOLOG_IS_LINUX
#include <ctime>
#else
#include <chrono>
#endif

namespace femtolog {

template <std::size_t N>
struct FixedString {
  char data[N + 1]{};
  std::size_t size = N;

  consteval FixedString(const char (&str)[N + 1]) {  // NOLINT
    for (std::size_t i = 0; i < N; ++i) {
      data[i] = str[i];
    }
    data[N] = '\0';
  }

  consteval std::string_view view() const {
    return std::string_view{data, size};
  }

  consteval const char* c_str() const { return data; }
};

template <std::size_t N>
FixedString(const char (&)[N]) -> FixedString<N - 1>;

inline uint64_t timestamp_ns() noexcept {
#if FEMTOLOG_IS_LINUX
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return static_cast<uint64_t>(ts.tv_sec) * 1000000000ull + ts.tv_nsec;
#else
  return static_cast<uint64_t>(
      std::chrono::steady_clock::now().time_since_epoch().count());
#endif
}

using FormatFunction = std::size_t (*)(fmt::memory_buffer*,
                                       const fmt::format_args&);

template <FixedString fmt>
struct FormatDispatcher {
  static std::size_t format(fmt::memory_buffer* buf,
                            const fmt::format_args& args) {
    auto result =
        fmt::vformat_to_n(buf->data(), buf->capacity(), fmt.view(), args);
    return result.size;
  }

  static constexpr FormatFunction function() { return &format; }
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_BASE_FORMAT_UTIL_H_
