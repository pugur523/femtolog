// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/core/diagnostics/signal_handler.h"
#include "femtolog/core/diagnostics/stack_trace.h"
#include "femtolog/core/diagnostics/terminate_handler.h"
#include "gtest/gtest.h"

namespace {

void init_tests() {
  testing::InitGoogleTest();
}

int run_tests() {
  return RUN_ALL_TESTS();
}

}  // namespace

[[clang::xray_always_instrument]]
int main() {
  femtolog::core::register_signal_handlers();
  femtolog::core::register_terminate_handler();
  femtolog::core::register_stack_trace_handler();
  init_tests();
  return run_tests();
}
