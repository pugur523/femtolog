// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef BENCH_BENCHMARK_UTIL_H_
#define BENCH_BENCHMARK_UTIL_H_

#include <string>

#include "femtolog/core/base/file_util.h"

namespace femtolog::bench {

inline std::string get_benchmark_log_path(const char* filename) {
  return core::join_path(core::exe_dir(), "benchmark_logs", filename);
}

}  // namespace femtolog::bench

#endif  // BENCH_BENCHMARK_UTIL_H_
