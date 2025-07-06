// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/backend_worker.h"

#include <limits>
#include <memory>
#include <utility>

#include "femtolog/sinks/null_sink.h"
#include "gtest/gtest.h"

namespace femtolog::logging {

TEST(BackendWorkerTest, RegisterAndClearSinks) {
  BackendWorker worker;
  auto sink = std::make_unique<NullSink>();
  FemtologOptions options;
  SpscQueue queue;
  StringRegistry registry;
  worker.init(&queue, &registry, options);

  worker.register_sink(std::move(sink));
  worker.clear_sinks();
}

TEST(BackendWorkerTest, CPUAffinityNoCrash) {
  BackendWorker worker;

  auto sink = std::make_unique<NullSink>();
  FemtologOptions options;
  options.backend_worker_cpu_affinity = 5;
  SpscQueue queue;
  StringRegistry registry;
  worker.init(&queue, &registry, options);
  worker.register_sink(std::move(sink));
  worker.start();
  worker.stop();
}

}  // namespace femtolog::logging
