// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <atomic>
#include <chrono>
#include <thread>

#include "benchmark/benchmark.h"
#include "femtolog/logging/impl/spsc_queue.h"

namespace femtolog::logging {

namespace {

constexpr std::size_t kDefaultQueueCapacity = 4096;           // 4KiB
constexpr std::size_t kMediumQueueCapacity = 65536;           // 64KiB
constexpr std::size_t kLargeQueueCapacity = 1024 * 1024 * 2;  // 2MiB

void spsc_queue_enqueue_1_byte(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kDefaultQueueCapacity);
  uint8_t data = 0xAA;
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(&data));
  }
}
BENCHMARK(spsc_queue_enqueue_1_byte);

void spsc_queue_dequeue_1_byte(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kDefaultQueueCapacity);
  uint8_t data = 0xAA;
  for (int i = 0; i < state.range(0); ++i) {
    queue.enqueue_bytes(&data);
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(&data));
  }
}
BENCHMARK(spsc_queue_dequeue_1_byte)->Range(8, kDefaultQueueCapacity / 2);

void spsc_queue_enqueue_16_bytes(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kDefaultQueueCapacity);
  uint8_t data[16] = {0};
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_enqueue_16_bytes);

void spsc_queue_dequeue_16_bytes(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kDefaultQueueCapacity);
  uint8_t data[16] = {0};
  for (std::size_t i = 0; i < state.range(0) / sizeof(data); ++i) {
    queue.enqueue_bytes(data, sizeof(data));
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_dequeue_16_bytes)->Range(32, kDefaultQueueCapacity / 2);

void spsc_queue_enqueue_64_bytes(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kDefaultQueueCapacity);
  uint8_t data[64] = {0};
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_enqueue_64_bytes);

void spsc_queue_dequeue_64_bytes(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kDefaultQueueCapacity);

  uint8_t data[64] = {0};
  for (std::size_t i = 0; i < state.range(0) / sizeof(data); ++i) {
    queue.enqueue_bytes(data, sizeof(data));
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_dequeue_64_bytes)->Range(128, kDefaultQueueCapacity / 2);

void spsc_queue_enqueue_peek_dequeue_16_bytes(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kDefaultQueueCapacity);
  uint8_t data_in[16] = {0};
  uint8_t data_out[16] = {0};
  for (auto _ : state) {
    queue.enqueue_bytes(data_in, sizeof(data_in));
    queue.peek_bytes(data_out, sizeof(data_out));
    benchmark::DoNotOptimize(queue.dequeue_bytes(data_out, sizeof(data_out)));
  }
}
BENCHMARK(spsc_queue_enqueue_peek_dequeue_16_bytes);

void spsc_queue_enqueue_1_byte_medium_capacity(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kMediumQueueCapacity);
  uint8_t data = 0xAA;
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(&data));
  }
}
BENCHMARK(spsc_queue_enqueue_1_byte_medium_capacity);

void spsc_queue_dequeue_1_byte_medium_capacity(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kMediumQueueCapacity);
  uint8_t data = 0xAA;
  for (int i = 0; i < state.range(0); ++i) {
    queue.enqueue_bytes(&data);
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(&data));
  }
}
BENCHMARK(spsc_queue_dequeue_1_byte_medium_capacity)
    ->Range(8, kMediumQueueCapacity / 2);

void spsc_queue_enqueue_64_bytes_medium_capacity(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kMediumQueueCapacity);
  uint8_t data[64] = {0};
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_enqueue_64_bytes_medium_capacity);

void spsc_queue_dequeue_64_bytes_medium_capacity(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kMediumQueueCapacity);
  uint8_t data[64] = {0};
  for (std::size_t i = 0; i < state.range(0) / sizeof(data); ++i) {
    queue.enqueue_bytes(data, sizeof(data));
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_dequeue_64_bytes_medium_capacity)
    ->Range(128, kMediumQueueCapacity / 2);

void spsc_queue_enqueue_1_byte_large_capacity(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kLargeQueueCapacity);
  uint8_t data = 0xAA;
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(&data));
  }
}
BENCHMARK(spsc_queue_enqueue_1_byte_large_capacity);

void spsc_queue_dequeue_1_byte_large_capacity(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kLargeQueueCapacity);
  uint8_t data = 0xAA;
  for (int i = 0; i < state.range(0); ++i) {
    queue.enqueue_bytes(&data);
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(&data));
  }
}
BENCHMARK(spsc_queue_dequeue_1_byte_large_capacity)
    ->Range(8, kLargeQueueCapacity / 2);

void spsc_queue_enqueue_64_bytes_large_capacity(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kLargeQueueCapacity);
  uint8_t data[64] = {0};
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_enqueue_64_bytes_large_capacity);

void spsc_queue_dequeue_64_bytes_large_capacity(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kLargeQueueCapacity);
  uint8_t data[64] = {0};
  for (std::size_t i = 0; i < state.range(0) / sizeof(data); ++i) {
    queue.enqueue_bytes(data, sizeof(data));
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(data, sizeof(data)));
  }
}
BENCHMARK(spsc_queue_dequeue_64_bytes_large_capacity)
    ->Range(128, kLargeQueueCapacity / 2);

void spsc_queue_consumer_bench_with_busy_producer(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kLargeQueueCapacity);
  std::atomic<bool> running{true};
  uint8_t data = 0xAA;

  // producer thread: keep enqueueing
  std::thread producer([&] {
    while (running.load(std::memory_order_relaxed)) {
      queue.enqueue_bytes(&data);
    }
  });

  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.dequeue_bytes(&data));
  }

  running = false;
  producer.join();
}
BENCHMARK(spsc_queue_consumer_bench_with_busy_producer);

void spsc_queue_producer_bench_with_busy_consumer(benchmark::State& state) {
  SpscQueue queue;
  queue.reserve(kLargeQueueCapacity);
  std::atomic<bool> running{true};
  uint8_t data = 0xAA;

  // producer thread: keep dequeueing
  std::thread consumer([&] {
    while (running.load(std::memory_order_relaxed)) {
      queue.dequeue_bytes(&data);
    }
  });

  for (auto _ : state) {
    benchmark::DoNotOptimize(queue.enqueue_bytes(&data));
  }

  running = false;
  consumer.join();
}
BENCHMARK(spsc_queue_producer_bench_with_busy_consumer);

}  // namespace

}  // namespace femtolog::logging
