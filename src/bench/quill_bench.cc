// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "benchmark/benchmark.h"
#include "logging/impl/internal_logger.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/NullSink.h"

#pragma clang diagnostic pop

namespace spdlog {

namespace {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"

constexpr femtolog::FemtologOptions default_options;
struct OptimizedFrontendOptions {
  static constexpr quill::QueueType queue_type =
      quill::QueueType::UnboundedDropping;
  static constexpr std::size_t initial_queue_capacity =
      default_options.spsc_queue_size;
  static constexpr uint32_t blocking_queue_retry_interval_ns = 800;
  static constexpr std::size_t unbounded_queue_max_capacity =
      default_options.spsc_queue_size;
  static constexpr quill::HugePagesPolicy huge_pages_policy =
      quill::HugePagesPolicy::Never;
};
#pragma clang diagnostic pop

using OptimizedFrontend = quill::FrontendImpl<OptimizedFrontendOptions>;
using OptimizedLogger = quill::LoggerImpl<OptimizedFrontendOptions>;

void quill_info_literal(benchmark::State& state) {
  quill::BackendOptions backend_options;
  backend_options.cpu_affinity = 4;
  backend_options.sleep_duration = std::chrono::nanoseconds{0};
  quill::Backend::start(backend_options);
  std::this_thread::sleep_for(std::chrono::nanoseconds(1));

  OptimizedLogger* logger = OptimizedFrontend::create_or_get_logger(
      "root", OptimizedFrontend::create_or_get_sink<quill::NullSink>("null"));

  for (auto _ : state) {
    LOG_INFO(logger, "Benchmark test message");
  }
  quill::Backend::stop();
}
BENCHMARK(quill_info_literal);

void quill_info_format(benchmark::State& state) {
  quill::BackendOptions backend_options;
  backend_options.cpu_affinity = 4;
  backend_options.sleep_duration = std::chrono::nanoseconds{0};
  quill::Backend::start(backend_options);
  std::this_thread::sleep_for(std::chrono::nanoseconds(1));

  OptimizedLogger* logger = OptimizedFrontend::create_or_get_logger(
      "root", OptimizedFrontend::create_or_get_sink<quill::NullSink>("null"));

  for (auto _ : state) {
    LOG_INFO(logger, "Benchmark test message {}", 123);
  }
  quill::Backend::stop();
}
BENCHMARK(quill_info_format);

}  // namespace

}  // namespace spdlog

