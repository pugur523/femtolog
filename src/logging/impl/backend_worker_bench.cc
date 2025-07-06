// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <limits>
#include <memory>
#include <utility>

#include "benchmark/benchmark.h"
#include "femtolog/logging/impl/backend_worker.h"
#include "femtolog/sinks/null_sink.h"

namespace femtolog::logging {

namespace {

void backend_worker_run_loop(benchmark::State& state) {
  BackendWorker worker;
  auto sink = std::make_unique<NullSink>();
  FemtologOptions options;
  options.backend_dequeue_buffer_size = 4096;
  options.backend_format_buffer_size = 4096;
  options.backend_worker_cpu_affinity = std::numeric_limits<std::size_t>::max();

  SpscQueue queue;
  StringRegistry registry;
  worker.init(&queue, &registry, options);
  worker.register_sink(std::move(sink));

  for (auto _ : state) {
    worker.start();
    worker.stop();
  }
}

BENCHMARK(backend_worker_run_loop);

}  // namespace

}  // namespace femtolog::logging
