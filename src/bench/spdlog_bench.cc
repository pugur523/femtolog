// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <memory>

#include "benchmark/benchmark.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/spdlog.h"

namespace spdlog {

namespace {

void spdlog_info_literal(benchmark::State& state) {
  auto null_sink = std::make_shared<sinks::null_sink_mt>();
  auto l = std::make_shared<logger>("spd", null_sink);
  set_default_logger(l);
  set_level(level::info);

  for (auto _ : state) {
    SPDLOG_INFO("Benchmark test message");
  }
}
BENCHMARK(spdlog_info_literal);

void spdlog_info_format(benchmark::State& state) {
  auto null_sink = std::make_shared<sinks::null_sink_mt>();
  auto l = std::make_shared<logger>("spd", null_sink);
  set_default_logger(l);
  set_level(level::info);

  for (auto _ : state) {
    SPDLOG_INFO("Benchmark test message {}", 123);
  }
}
BENCHMARK(spdlog_info_format);

}  // namespace

}  // namespace spdlog
