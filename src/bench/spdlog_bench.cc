// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "bench/benchmark_util.h"
#include "benchmark/benchmark.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/spdlog.h"

namespace spdlog {

namespace {

std::shared_ptr<logger> setup_logger() {
  auto null_sink = std::make_shared<sinks::null_sink_mt>();
  auto l = std::make_shared<logger>("spd", null_sink);
  // auto file_sink = std::make_shared<sinks::basic_file_sink_mt>(
  //     femtolog::bench::get_benchmark_log_path("spdlog.log"));
  // auto l = std::make_shared<logger>("spd", file_sink);
  set_default_logger(l);
  set_level(level::info);

  std::this_thread::sleep_for(std::chrono::milliseconds(25));
  return l;
}

void spdlog_info_literal(benchmark::State& state) {
  auto l = setup_logger();
  for (auto _ : state) {
    SPDLOG_INFO("Benchmark test message");
  }
}
BENCHMARK(spdlog_info_literal);

void spdlog_info_format_int(benchmark::State& state) {
  auto l = setup_logger();
  for (auto _ : state) {
    SPDLOG_INFO("Value: {}", 123);
  }
}
BENCHMARK(spdlog_info_format_int);

void spdlog_info_format_multi_int(benchmark::State& state) {
  auto l = setup_logger();
  for (auto _ : state) {
    SPDLOG_INFO("A: {}, B: {}, C: {}", 1, 2, 3);
  }
}
BENCHMARK(spdlog_info_format_multi_int);

void spdlog_info_format_small_string(benchmark::State& state) {
  auto l = setup_logger();
  std::string user = "benchmark_user";
  for (auto _ : state) {
    SPDLOG_INFO("User: {}", user);
  }
}
BENCHMARK(spdlog_info_format_small_string);

void spdlog_info_format_small_string_view(benchmark::State& state) {
  auto l = setup_logger();
  std::string_view sv = "benchmark_view";
  for (auto _ : state) {
    SPDLOG_INFO("View: {}", sv);
  }
}
BENCHMARK(spdlog_info_format_small_string_view);

void spdlog_info_format_mixed(benchmark::State& state) {
  auto l = setup_logger();
  std::string user = "user42";
  std::string_view op = "login";
  bool success = true;
  int64_t id = 9876543210;
  for (auto _ : state) {
    SPDLOG_INFO("User: {}, Op: {}, Success: {}, ID: {}", user, op, success, id);
  }
}
BENCHMARK(spdlog_info_format_mixed);

void spdlog_info_format_large_string(benchmark::State& state) {
  auto l = setup_logger();
  std::string payload(512, 'X');
  for (auto _ : state) {
    SPDLOG_INFO("Payload: {}", payload);
  }
}
BENCHMARK(spdlog_info_format_large_string);

}  // namespace

}  // namespace spdlog
