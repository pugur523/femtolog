// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "benchmark/benchmark.h"
#include "logging/impl/spsc_queue.h"

namespace femtolog::logging {

namespace {

constexpr std::size_t kQueueCapacity = 4095;  // 4KiB

void spsc_queue_enqueue_1_byte(benchmark::State& state) {
  SpscQueue queue(kQueueCapacity);
  uint8_t data = 0xAA;
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(&data));
  }
}
BENCHMARK(spsc_queue_enqueue_1_byte);

void spsc_queue_dequeue_1_byte(benchmark::State& state) {
  SpscQueue queue(kQueueCapacity);
  uint8_t data = 0xAA;
  for (int i = 0; i < state.range(0); ++i) {
    queue.enqueue_bytes(&data);
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(&data));
  }
}
BENCHMARK(spsc_queue_dequeue_1_byte)->Range(8, kQueueCapacity / 2);

void spsc_queue_enqueue_16_bytes(benchmark::State& state) {
  SpscQueue queue(kQueueCapacity);
  uint8_t data[16] = {0};
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_enqueue_16_bytes);

void spsc_queue_dequeue_16_bytes(benchmark::State& state) {
  SpscQueue queue(kQueueCapacity);
  uint8_t data[16] = {0};
  for (std::size_t i = 0; i < state.range(0) / sizeof(data); ++i) {
    queue.enqueue_bytes(data, sizeof(data));
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_dequeue_16_bytes)->Range(32, kQueueCapacity / 2);

void spsc_queue_enqueue_64_bytes(benchmark::State& state) {
  SpscQueue queue(kQueueCapacity);
  uint8_t data[64] = {0};
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_enqueue_64_bytes);

void spsc_queue_dequeue_64_bytes(benchmark::State& state) {
  SpscQueue queue(kQueueCapacity);
  uint8_t data[64] = {0};
  for (std::size_t i = 0; i < state.range(0) / sizeof(data); ++i) {
    queue.enqueue_bytes(data, sizeof(data));
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_dequeue_64_bytes)->Range(128, kQueueCapacity / 2);

void spsc_queue_enqueue_peek_dequeue_16_bytes(benchmark::State& state) {
  SpscQueue queue(kQueueCapacity);
  uint8_t data_in[16] = {0};
  uint8_t data_out[16] = {0};
  for (auto _ : state) {
    queue.enqueue_bytes(data_in, sizeof(data_in));
    queue.peek_bytes(data_out, sizeof(data_out));
    benchmark::DoNotOptimize(queue.dequeue_bytes(data_out, sizeof(data_out)));
  }
}
BENCHMARK(spsc_queue_enqueue_peek_dequeue_16_bytes);

}  // namespace

}  // namespace femtolog::logging
