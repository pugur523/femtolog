// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/femtolog.h"
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
  femtolog::register_signal_handlers();
  femtolog::register_terminate_handler();
  femtolog::register_stack_trace_handler();
  init_tests();
  return run_tests();
}
