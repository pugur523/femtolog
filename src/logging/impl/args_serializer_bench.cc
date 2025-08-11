// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>

#include "benchmark/benchmark.h"
#include "femtolog/logging/impl/args_serializer.h"

namespace femtolog::logging {

namespace {

void args_serializer_serialize_ints(benchmark::State& state) {
  ArgsSerializer serializer;
  for (auto _ : state) {
    benchmark::DoNotOptimize(serializer.serialize<"", false>(1, 2, 3, 4, 5));
  }
}
BENCHMARK(args_serializer_serialize_ints);

void args_serializer_serialize_strings(benchmark::State& state) {
  ArgsSerializer serializer;
  const char* a = "hello";
  std::string_view b = "world!";
  for (auto _ : state) {
    benchmark::DoNotOptimize(serializer.serialize<"", false>(a, b));
  }
}
BENCHMARK(args_serializer_serialize_strings);

void args_serializer_serialize_mixed(benchmark::State& state) {
  ArgsSerializer serializer;
  std::string_view s = "mixed";
  for (auto _ : state) {
    benchmark::DoNotOptimize(
        serializer.serialize<"", false>(42, true, s, 3.14));
  }
}
BENCHMARK(args_serializer_serialize_mixed);

}  // namespace

}  // namespace femtolog::logging
