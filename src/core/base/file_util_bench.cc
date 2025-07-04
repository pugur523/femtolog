// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <random>
#include <string>
#include <vector>

#include "benchmark/benchmark.h"
#include "core/base/file_util.h"

namespace core {

namespace {

// Setup temporary test files/directories
template <typename F>
void with_temp_file(const std::string& content, const F& fn) {
  TempFile tmp("temp_file_for_bench", content);
  fn(tmp.path());
}

template <typename F>
void with_temp_dir(const F& fn) {
  TempDir tmp;
  fn(tmp.path());
}

void file_util_file_exists(benchmark::State& state) {
  with_temp_file("hello", [&](const std::string& path) {
    for (auto _ : state) {
      benchmark::DoNotOptimize(file_exists(path.c_str()));
    }
  });
}
BENCHMARK(file_util_file_exists);

void file_util_dir_exists(benchmark::State& state) {
  with_temp_dir([&](const std::string& path) {
    for (auto _ : state) {
      benchmark::DoNotOptimize(dir_exists(path.c_str()));
    }
  });
}
BENCHMARK(file_util_dir_exists);

void file_util_exe_path(benchmark::State& state) {
  for (auto _ : state) {
    std::string this_path = exe_path();
    benchmark::DoNotOptimize(this_path);
  }
}
BENCHMARK(file_util_exe_path);

void file_util_exe_dir(benchmark::State& state) {
  for (auto _ : state) {
    std::string this_dir = exe_dir();
    benchmark::DoNotOptimize(this_dir);
  }
}
BENCHMARK(file_util_exe_dir);

void file_util_list_files(benchmark::State& state) {
  with_temp_dir([&](const std::string& dir) {
    for (int i = 0; i < 10; ++i) {
      const std::string path =
          join_path(dir, std::string("file" + std::to_string(i)));
      create_file(path.c_str());
      write_file(path.c_str(), "test");
    }
    for (auto _ : state) {
      benchmark::DoNotOptimize(list_files(dir));
    }

    for (int i = 0; i < 10; ++i) {
      const std::string path =
          join_path(dir, std::string("file" + std::to_string(i)));
      remove_file(path.c_str());
    }
  });
}
BENCHMARK(file_util_list_files);

void file_util_write_binary_to_file(benchmark::State& state) {
  with_temp_file("", [&](const std::string& path) {
    std::vector<char> data(4096, 'A');
    for (auto _ : state) {
      write_binary_to_file(data.data(), data.size(), path);
    }
  });
}
BENCHMARK(file_util_write_binary_to_file);

void file_util_file_extension(benchmark::State& state) {
  const std::string path = "/path/to/file.name.ext";
  for (auto _ : state) {
    benchmark::DoNotOptimize(file_extension(path));
  }
}
BENCHMARK(file_util_file_extension);

void file_util_sanitize_component(benchmark::State& state) {
  const char* path = "/foo/bar/";
  for (auto _ : state) {
    benchmark::DoNotOptimize(sanitize_component(path, false));
  }
}
BENCHMARK(file_util_sanitize_component);

void file_util_remove_file(benchmark::State& state) {
  const std::string tmp = temp_path("temp_bench_remove_file");
  for (auto _ : state) {
    create_file(tmp.c_str());
    write_file(tmp.c_str(), "test");
    benchmark::DoNotOptimize(remove_file(tmp.c_str()));
  }
}
BENCHMARK(file_util_remove_file);

void file_util_rename_file(benchmark::State& state) {
  const std::string temp_dir = temp_directory();
  const std::string new_name = join_path(temp_dir, "tmp_new_name.txt");
  for (auto _ : state) {
    const std::string old_name = temp_path("temp_bench_rename_file");
    create_file(old_name.c_str());
    write_file(old_name.c_str(), "test");
    rename_file(old_name.c_str(), new_name.c_str());
    remove_file(new_name.c_str());
  }
}
BENCHMARK(file_util_rename_file);

void file_util_create_directory(benchmark::State& state) {
  with_temp_dir([&](const std::string& temp_dir) {
    const std::string dir = join_path(temp_dir, "temp_bench_create_directory");
    for (auto _ : state) {
      create_directory(dir.c_str());
      remove_directory(dir.c_str());
    }
  });
}
BENCHMARK(file_util_create_directory);

std::string generate_large_content(std::size_t num_lines,
                                   std::size_t line_length) {
  std::string content;
  content.reserve(num_lines * (line_length + 1));  // +1 for newline
  for (std::size_t i = 0; i < num_lines; ++i) {
    for (std::size_t j = 0; j < line_length; ++j) {
      content += 'a' + (j % 26);  // a-z
    }
    content += '\n';
  }
  return content;
}

template <bool use_avx>
void file_util_read_lines_internal(benchmark::State& state) {
  const std::size_t num_lines = 1000;
  const std::size_t line_length = 80;
  const std::string large_content =
      generate_large_content(num_lines, line_length);

  for (auto _ : state) {
    std::vector<std::string> lines = read_lines<use_avx>(large_content);
    benchmark::DoNotOptimize(lines);
  }
  state.SetBytesProcessed(state.iterations() * large_content.size());
}

void file_util_read_lines_default(benchmark::State& state) {
  file_util_read_lines_internal<false>(state);
}
BENCHMARK(file_util_read_lines_default);

void file_util_read_lines_with_avx2(benchmark::State& state) {
  file_util_read_lines_internal<true>(state);
}
BENCHMARK(file_util_read_lines_with_avx2);

void file_util_file_constructor(benchmark::State& state) {
  const std::size_t num_lines = 1000;
  const std::size_t line_length = 80;
  const std::string file_name = "test_file.txt";
  const std::string large_content =
      generate_large_content(num_lines, line_length);

  for (auto _ : state) {
    File f = File(std::string(file_name), std::string(large_content));
    benchmark::DoNotOptimize(f);
  }
  state.SetBytesProcessed(state.iterations() *
                          (file_name.size() + large_content.size()));
}
BENCHMARK(file_util_file_constructor);

void file_util_file_line_access_random(benchmark::State& state) {
  const std::size_t num_lines = 1000;
  const std::size_t line_length = 80;
  const std::string large_content =
      generate_large_content(num_lines, line_length);
  File f("test_file.txt", std::string(large_content));

  std::mt19937 gen(std::random_device{}());  // NOLINT
  std::uniform_int_distribution<> distrib(1, num_lines);

  for (auto _ : state) {
    std::size_t line_no = distrib(gen);
    f.line(line_no);
  }
  state.SetBytesProcessed(state.iterations() * line_length);
}
BENCHMARK(file_util_file_line_access_random);

void file_util_file_line_access_sequential(benchmark::State& state) {
  const std::size_t num_lines = 1000;
  const std::size_t line_length = 80;
  const std::string large_content =
      generate_large_content(num_lines, line_length);
  File f("test_file.txt", std::string(large_content));

  std::size_t current_line = 1;

  for (auto _ : state) {
    f.line(current_line);

    current_line++;
    if (current_line > num_lines) {
      current_line = 1;
    }
  }
  state.SetBytesProcessed(state.iterations() * line_length);
}
BENCHMARK(file_util_file_line_access_sequential);

void file_util_file_source_access(benchmark::State& state) {
  const std::size_t num_lines = 1000;
  const std::size_t line_length = 80;
  const std::string large_content =
      generate_large_content(num_lines, line_length);
  File f("test_file.txt", std::string(large_content));

  for (auto _ : state) {
    f.source();
  }
  state.SetBytesProcessed(state.iterations() * large_content.size());
}
BENCHMARK(file_util_file_source_access);

void file_util_file_name_access(benchmark::State& state) {
  const std::string file_name = "test_file.txt";
  File f(std::string(file_name), "");

  for (auto _ : state) {
    f.file_name();
  }
  state.SetBytesProcessed(state.iterations() * file_name.size());
}
BENCHMARK(file_util_file_name_access);

}  // namespace

}  // namespace core
