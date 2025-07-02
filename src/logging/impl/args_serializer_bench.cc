// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>

#include "benchmark/benchmark.h"
#include "logging/impl/args_serializer.h"

namespace femtolog::logging {

static void args_serializer_serialize_ints(benchmark::State& state) {
  DefaultSerializer serializer;
  for (auto _ : state) {
    benchmark::DoNotOptimize(serializer.serialize(1, 2, 3, 4, 5));
  }
}
BENCHMARK(args_serializer_serialize_ints);

static void args_serializer_serialize_strings(benchmark::State& state) {
  DefaultSerializer serializer;
  const char* a = "hello";
  std::string_view b = "world!";
  for (auto _ : state) {
    benchmark::DoNotOptimize(serializer.serialize(a, b));
  }
}
BENCHMARK(args_serializer_serialize_strings);

static void args_serializer_serialize_mixed(benchmark::State& state) {
  DefaultSerializer serializer;
  std::string_view s = "mixed";
  for (auto _ : state) {
    benchmark::DoNotOptimize(serializer.serialize(42, true, s, 3.14));
  }
}
BENCHMARK(args_serializer_serialize_mixed);

static void args_serializer_serialize_empty(benchmark::State& state) {
  DefaultSerializer serializer;
  for (auto _ : state) {
    benchmark::DoNotOptimize(serializer.serialize());
  }
}
BENCHMARK(args_serializer_serialize_empty);

}  // namespace femtolog::logging
