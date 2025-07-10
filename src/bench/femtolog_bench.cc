// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>

#include "bench/benchmark_util.h"
#include "benchmark/benchmark.h"
#include "femtolog/logger.h"
#include "femtolog/options.h"
#include "femtolog/sinks/file_sink.h"
#include "femtolog/sinks/null_sink.h"
#include "femtolog/sinks/stdout_sink.h"

namespace femtolog {

namespace {

Logger& setup_logger() {
  static bool initialized = false;

  Logger& logger = Logger::logger();
  if (!initialized) {
    logger.init(kFastOptions);
    logger.register_sink<NullSink>();
    // logger.register_sink<FileSink>(
    //     bench::get_benchmark_log_path("femtolog.log"));

    initialized = true;
  }
  logger.start_worker();
  std::this_thread::sleep_for(std::chrono::milliseconds(25));

  return logger;
}

inline void summarize_result(const Logger& logger, benchmark::State& state) {
  state.counters["enqueued count"] = logger.enqueued_count();
  state.counters["dropped count"] = logger.dropped_count();
}

inline void teardown_logger(Logger* logger) {
  logger->stop_worker();
  logger->reset_count();
}

void femtolog_info_literal(benchmark::State& state) {
  Logger& logger = setup_logger();
  for (auto _ : state) {
    logger.info<"Benchmark test message\n">();
  }
  summarize_result(logger, state);
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_literal);

void femtolog_info_format_int(benchmark::State& state) {
  Logger& logger = setup_logger();
  for (auto _ : state) {
    logger.info<"Value: {}\n">(123);
  }
  summarize_result(logger, state);
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_format_int);

void femtolog_info_format_multi_int(benchmark::State& state) {
  Logger& logger = setup_logger();
  for (auto _ : state) {
    logger.info<"A: {}, B: {}, C: {}\n">(1, 2, 3);
  }
  summarize_result(logger, state);
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_format_multi_int);

void femtolog_info_format_small_string(benchmark::State& state) {
  Logger& logger = setup_logger();
  std::string name = "benchmark_user";
  for (auto _ : state) {
    logger.info<"User: {}\n">(name);
  }
  summarize_result(logger, state);
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_format_small_string);

// std::string_view argument
void femtolog_info_format_small_string_view(benchmark::State& state) {
  Logger& logger = setup_logger();
  std::string_view sv = "benchmark_view";
  for (auto _ : state) {
    logger.info<"View: {}\n">(sv);
  }
  summarize_result(logger, state);
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_format_small_string_view);

// Mixed types
void femtolog_info_format_mixed(benchmark::State& state) {
  Logger& logger = setup_logger();
  std::string user = "user42";
  std::string_view op = "login";
  bool success = true;
  int64_t id = 9876543210;
  for (auto _ : state) {
    logger.info<"User: {}, Op: {}, Success: {}, ID: {}\n">(user, op, success,
                                                           id);
  }
  summarize_result(logger, state);
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_format_mixed);

// Long string argument
void femtolog_info_format_large_string(benchmark::State& state) {
  Logger& logger = setup_logger();
  std::string long_str = std::string(64, 'X');  // 64-byte string
  for (auto _ : state) {
    logger.info<"Payload: {}\n">(long_str);
  }
  summarize_result(logger, state);
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_format_large_string);

}  // namespace

}  // namespace femtolog

