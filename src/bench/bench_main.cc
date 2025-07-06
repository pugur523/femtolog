// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "benchmark/benchmark.h"
#include "femtolog/core/diagnostics/signal_handler.h"
#include "femtolog/core/diagnostics/stack_trace.h"
#include "femtolog/core/diagnostics/terminate_handler.h"

[[clang::xray_always_instrument]]
int main(int argc, char** argv) {
  core::register_signal_handlers();
  core::register_terminate_handler();
  core::register_stack_trace_handler();

  benchmark ::MaybeReenterWithoutASLR(argc, argv);
  char arg0_default[] = "benchmark";
  char* args_default = reinterpret_cast<char*>(arg0_default);
  if (!argv) {
    argc = 1;
    argv = &args_default;
  }
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
