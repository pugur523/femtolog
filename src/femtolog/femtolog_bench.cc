// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>

#include "benchmark/benchmark.h"
#include "femtolog/logger.h"
#include "femtolog/sinks/null_sink.h"

namespace femtolog {

namespace {

Logger& setup_logger() {
  static bool initialized = false;

  Logger& logger = Logger::logger();
  if (!initialized) {
    logger.init();
    logger.register_sink<NullSink>();

    initialized = true;
  }
  logger.start_worker();
  std::this_thread::sleep_for(std::chrono::microseconds(1));

  return logger;
}

void femtolog_info_literal(benchmark::State& state) {
  Logger logger = setup_logger();
  for (auto _ : state) {
    logger.info<"Benchmark test message">();
  }

  logger.stop_worker();
}
BENCHMARK(femtolog_info_literal);

void femtolog_info_format_int(benchmark::State& state) {
  Logger logger = setup_logger();
  for (auto _ : state) {
    logger.info<"Value: {}">(123);
  }
  logger.stop_worker();
}
BENCHMARK(femtolog_info_format_int);

void femtolog_info_format_multi_int(benchmark::State& state) {
  Logger logger = setup_logger();
  for (auto _ : state) {
    logger.info<"A: {}, B: {}, C: {}">(1, 2, 3);
  }
  logger.stop_worker();
}
BENCHMARK(femtolog_info_format_multi_int);

void femtolog_info_format_string(benchmark::State& state) {
  Logger logger = setup_logger();
  std::string name = "benchmark_user";
  for (auto _ : state) {
    logger.info<"User: {}">(name);
  }
  logger.stop_worker();
}
BENCHMARK(femtolog_info_format_string);

// std::string_view argument
void femtolog_info_format_string_view(benchmark::State& state) {
  Logger logger = setup_logger();
  std::string_view sv = "benchmark_view";
  for (auto _ : state) {
    logger.info<"View: {}">(sv);
  }
  logger.stop_worker();
}
BENCHMARK(femtolog_info_format_string_view);

// Mixed types
void femtolog_info_format_mixed(benchmark::State& state) {
  Logger logger = setup_logger();
  std::string user = "user42";
  std::string_view op = "login";
  bool success = true;
  int64_t id = 9876543210;
  for (auto _ : state) {
    logger.info<"User: {}, Op: {}, Success: {}, ID: {}">(user, op, success, id);
  }
  logger.stop_worker();
}
BENCHMARK(femtolog_info_format_mixed);

// Long string argument
void femtolog_info_format_large_string(benchmark::State& state) {
  Logger logger = setup_logger();
  std::string long_str = std::string(64, 'X');  // 64-byte string
  for (auto _ : state) {
    logger.info<"Payload: {}">(long_str);
  }
  logger.stop_worker();
}
BENCHMARK(femtolog_info_format_large_string);

}  // namespace

}  // namespace femtolog

