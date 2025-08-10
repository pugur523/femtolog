// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>

#include "benchmark/benchmark.h"
#include "femtolog/logging/impl/args_deserializer.h"
#include "femtolog/logging/impl/args_serializer.h"

namespace femtolog::logging {

namespace {

void args_deserializer_deserialize_and_format(benchmark::State& state) {
  int x = 100;
  std::string y = "benchmark";
  double z = 9.99;

  ArgsSerializer<256> serializer;
  auto& args = serializer.serialize<"x={}, y={}, z={}">(x, y, z);

  const SerializedArgsHeader* header =
      reinterpret_cast<const SerializedArgsHeader*>(args.data());
  const char* payload =
      reinterpret_cast<const char*>(args.data() + sizeof(*header));

  fmt::memory_buffer buf;
  buf.reserve(1024);
  for (auto _ : state) {
    std::size_t n =
        header->deserialize_and_format_func(&buf, header->format_func, payload);
    // benchmark::DoNotOptimize(buf.data());
    benchmark::DoNotOptimize(n);
  }
}

BENCHMARK(args_deserializer_deserialize_and_format);

}  // namespace

}  // namespace femtolog::logging
