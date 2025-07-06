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
    FemtologOptions options;
    // use default options
    logger.init(options);
    // logger.register_sink<NullSink>();
    logger.register_sink<FileSink<>>(
        bench::get_benchmark_log_path("femtolog.log"));

    initialized = true;
  }
  logger.start_worker();
  std::this_thread::sleep_for(std::chrono::milliseconds(25));

  return logger;
}

Logger& glog() {
  static bool initialized = false;

  Logger& logger = Logger::global_logger();

  if (!initialized) {
    FemtologOptions default_opts;
    logger.init(default_opts);
    logger.register_sink<StdoutSink<>>();
    initialized = true;
  }
  logger.start_worker();
  std::this_thread::sleep_for(std::chrono::milliseconds(25));

  return logger;
}

void teardown_logger(Logger* logger) {
  logger->stop_worker();
  Logger& gl = glog();
  gl.info<"enqueued count: {}, dropped count: {}\n">(logger->enqueued_count(),
                                                     logger->dropped_count());
  logger->reset_count();
  gl.stop_worker();
}

void femtolog_info_literal(benchmark::State& state) {
  Logger& logger = setup_logger();
  for (auto _ : state) {
    logger.info<"Benchmark test message\n">();
  }

  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_literal);

void femtolog_info_format_int(benchmark::State& state) {
  Logger& logger = setup_logger();
  for (auto _ : state) {
    logger.info<"Value: {}\n">(123);
  }
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_format_int);

void femtolog_info_format_multi_int(benchmark::State& state) {
  Logger& logger = setup_logger();
  for (auto _ : state) {
    logger.info<"A: {}, B: {}, C: {}\n">(1, 2, 3);
  }
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_format_multi_int);

void femtolog_info_format_small_string(benchmark::State& state) {
  Logger& logger = setup_logger();
  std::string name = "benchmark_user";
  for (auto _ : state) {
    logger.info<"User: {}\n">(name);
  }
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
  teardown_logger(&logger);
}
BENCHMARK(femtolog_info_format_large_string);

}  // namespace

}  // namespace femtolog

