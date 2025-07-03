// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>

#include "benchmark/benchmark.h"
#include "femtolog/base/format_util.h"
#include "femtolog/options.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/NullSink.h"

#pragma clang diagnostic pop

namespace quill {

namespace {

constexpr femtolog::FemtologOptions default_options;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
struct OptimizedFrontendOptions {
  static constexpr QueueType queue_type = QueueType::UnboundedDropping;
  static constexpr std::size_t initial_queue_capacity =
      default_options.spsc_queue_size;
  static constexpr uint32_t blocking_queue_retry_interval_ns = 800;
  static constexpr std::size_t unbounded_queue_max_capacity =
      default_options.spsc_queue_size;
  static constexpr HugePagesPolicy huge_pages_policy = HugePagesPolicy::Never;
};
#pragma clang diagnostic pop

using OptimizedFrontend = FrontendImpl<OptimizedFrontendOptions>;
using OptimizedLogger = LoggerImpl<OptimizedFrontendOptions>;

OptimizedLogger* setup_logger() {
  BackendOptions backend_options;
  backend_options.cpu_affinity = 4;
  backend_options.sleep_duration = std::chrono::nanoseconds{0};
  Backend::start(backend_options);
  std::this_thread::sleep_for(std::chrono::microseconds(1));

  return OptimizedFrontend::create_or_get_logger(
      "root", OptimizedFrontend::create_or_get_sink<NullSink>("null"));
}

void teardown_logger() {
  Backend::stop();
}

void quill_info_literal(benchmark::State& state) {
  OptimizedLogger* logger = setup_logger();

  for (auto _ : state) {
    LOG_INFO(logger, "Benchmark test message");
  }
  teardown_logger();
}
BENCHMARK(quill_info_literal);

void quill_info_format_int(benchmark::State& state) {
  OptimizedLogger* logger = setup_logger();
  for (auto _ : state) {
    LOG_INFO(logger, "Value: {}", 123);
  }
  teardown_logger();
}
BENCHMARK(quill_info_format_int);

void quill_info_format_multi_int(benchmark::State& state) {
  OptimizedLogger* logger = setup_logger();
  for (auto _ : state) {
    LOG_INFO(logger, "A: {}, B: {}, C: {}", 1, 2, 3);
  }
  teardown_logger();
}
BENCHMARK(quill_info_format_multi_int);

void quill_info_format_string(benchmark::State& state) {
  OptimizedLogger* logger = setup_logger();
  std::string user = "benchmark_user";
  for (auto _ : state) {
    LOG_INFO(logger, "User: {}", user);
  }
  teardown_logger();
}
BENCHMARK(quill_info_format_string);

void quill_info_format_string_view(benchmark::State& state) {
  OptimizedLogger* logger = setup_logger();
  std::string_view sv = "benchmark_view";
  for (auto _ : state) {
    LOG_INFO(logger, "View: {}", sv);
  }
  teardown_logger();
}
BENCHMARK(quill_info_format_string_view);

void quill_info_format_mixed(benchmark::State& state) {
  OptimizedLogger* logger = setup_logger();
  std::string user = "user42";
  std::string_view op = "login";
  bool success = true;
  int64_t id = 9876543210;
  for (auto _ : state) {
    LOG_INFO(logger, "User: {}, Op: {}, Success: {}, ID: {}", user, op, success,
             id);
  }
  teardown_logger();
}
BENCHMARK(quill_info_format_mixed);

void quill_info_format_large_string(benchmark::State& state) {
  OptimizedLogger* logger = setup_logger();
  std::string payload(64, 'X');
  for (auto _ : state) {
    LOG_INFO(logger, "Payload: {}", payload);
  }
  teardown_logger();
}
BENCHMARK(quill_info_format_large_string);

}  // namespace

}  // namespace quill
