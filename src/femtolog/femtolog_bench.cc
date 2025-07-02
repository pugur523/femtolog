// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "benchmark/benchmark.h"
#include "femtolog/logger.h"
#include "femtolog/sinks/null_sink.h"

namespace femtolog {

namespace {

void femtolog_info_literal(benchmark::State& state) {
  Logger logger = Logger::logger();
  logger.init();
  logger.register_sink<NullSink>();

  logger.start_worker();

  for (auto _ : state) {
    logger.info<"Benchmark test message">();
  }

  logger.stop_worker();
}
BENCHMARK(femtolog_info_literal);

void femtolog_info_format(benchmark::State& state) {
  Logger logger = Logger::logger();
  logger.init();
  logger.register_sink<NullSink>();

  logger.start_worker();

  for (auto _ : state) {
    logger.info<"Benchmark test message {}">(123);
  }

  logger.stop_worker();
}
BENCHMARK(femtolog_info_format);

}  // namespace

}  // namespace femtolog

