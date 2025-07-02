// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <array>
#include <format>
#include <string>
#include <string_view>

#include "benchmark/benchmark.h"
#include "fmt/core.h"
#include "fmt/format.h"

namespace {

constexpr const int value = 42;
constexpr const std::string_view name = "Foo";
constexpr const double pi = 3.14159;

void format_std_format_simple(benchmark::State& state) {
  for (auto _ : state) {
    std::string s = std::format("value = {}", value);
    benchmark::DoNotOptimize(s);
  }
}
BENCHMARK(format_std_format_simple);

void format_fmt_format_simple(benchmark::State& state) {
  for (auto _ : state) {
    std::string s = fmt::format("value = {}", value);
    benchmark::DoNotOptimize(s);
  }
}
BENCHMARK(format_fmt_format_simple);

void format_std_format(benchmark::State& state) {
  for (auto _ : state) {
    std::string s =
        std::format("value = {}, name = {}, pi = {:.2f}", value, name, pi);
    benchmark::DoNotOptimize(s);
  }
}
BENCHMARK(format_std_format);

void format_fmt_format(benchmark::State& state) {
  for (auto _ : state) {
    std::string s =
        fmt::format("value = {}, name = {}, pi = {:.2f}", value, name, pi);
    benchmark::DoNotOptimize(s);
  }
}
BENCHMARK(format_fmt_format);

void format_std_format_to_n_simple(benchmark::State& state) {
  for (auto _ : state) {
    std::array<char, 128> buffer;
    auto result =
        std::format_to_n(buffer.data(), buffer.size(), "value = {}", value);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(format_std_format_to_n_simple);

void format_fmt_format_to_n_simple(benchmark::State& state) {
  for (auto _ : state) {
    std::array<char, 128> buffer;
    auto result = fmt::format_to_n(buffer.data(), buffer.size(),
                                   FMT_STRING("value = {}"), value);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(format_fmt_format_to_n_simple);

void format_fmt_format_to_n_simple_wo_fmt_string(benchmark::State& state) {
  for (auto _ : state) {
    std::array<char, 128> buffer;
    auto result =
        fmt::format_to_n(buffer.data(), buffer.size(), "value = {}", value);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(format_fmt_format_to_n_simple_wo_fmt_string);

void format_std_format_to_n(benchmark::State& state) {
  for (auto _ : state) {
    std::array<char, 128> buffer;
    auto result =
        std::format_to_n(buffer.data(), buffer.size(),
                         "value = {}, name = {}, pi = {:.2f}", value, name, pi);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(format_std_format_to_n);

void format_fmt_format_to_n(benchmark::State& state) {
  for (auto _ : state) {
    std::array<char, 128> buffer;
    auto result = fmt::format_to_n(
        buffer.data(), buffer.size(),
        FMT_STRING("value = {}, name = {}, pi = {:.2f}"), value, name, pi);
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(format_fmt_format_to_n);

// void format_std_format_dynamic(benchmark::State& state) {
//   const char* format_str = "value = {}, name = {}, pi = {:.2f}";
//   for (auto _ : state) {
//     std::string s = std::format(format_str, value, name, pi);
//     benchmark::DoNotOptimize(s);
//   }
// }
// BENCHMARK(format_std_format_dynamic);

void format_fmt_format_dynamic(benchmark::State& state) {
  const char* format_str = "value = {}, name = {}, pi = {:.2f}";
  for (auto _ : state) {
    std::string s = fmt::format(fmt::runtime(format_str), value, name, pi);
    benchmark::DoNotOptimize(s);
  }
}
BENCHMARK(format_fmt_format_dynamic);

}  // namespace
