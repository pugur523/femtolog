// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "benchmark/benchmark.h"
#include "femtolog/logging/impl/internal_logger.h"
#include "femtolog/sinks/null_sink.h"

namespace femtolog::logging {

namespace {

void internal_logger_literal_log(benchmark::State& state) {
  InternalLogger logger;
  logger.init();
  logger.register_sink(std::make_unique<NullSink>());
  logger.start_worker();

  for (auto _ : state) {
    logger.log<LogLevel::kInfo, "Benchmark literal", false>();
  }

  logger.stop_worker();
}
BENCHMARK(internal_logger_literal_log);

void internal_logger_formatted_log(benchmark::State& state) {
  InternalLogger logger;
  logger.init();
  logger.register_sink(std::make_unique<NullSink>());
  logger.start_worker();

  const char* str = "times";

  for (auto _ : state) {
    logger.log<LogLevel::kInfo, "Benchmark {} {}", false>(42, str);
  }

  logger.stop_worker();
}
BENCHMARK(internal_logger_formatted_log);

}  // namespace

}  // namespace femtolog::logging
