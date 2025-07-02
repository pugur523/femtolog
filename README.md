<h1 align=center>
  femtolog
</h1>

[![CI](https://github.com/pugur523/femtolog/actions/workflows/ci.yml/badge.svg)](https://github.com/pugur523/femtolog/actions/workflows/ci.yml)
[![Issues](https://img.shields.io/github/issues/pugur523/femtolog.svg)](https://github.com/pugur523/femtolog/issues)
[![License](https://img.shields.io/badge/License-Apache%20License%20Version%202.0-red)](LICENSE)
[![C++20](https://img.shields.io/badge/C++-20-blue?logo=cplusplus)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-4.0.2+-green?logo=cmake)](https://cmake.org/)
<br/>
[![日本語の説明はこちら](https://img.shields.io/badge/日本語の説明はこちら-blue)](README_ja.md)

> Ultra-Light, High-Performance Asynchronous Logger

## ☄ Overview

**femtolog** is a blazing-fast, minimal-overhead asynchronous logging library built for performance-critical applications. It leverages zero-cost abstractions, cache-aligned SPSC queues, and compile-time format string serialization.

Designed for modern C++ projects where every nanosecond counts.

---

## 📖 Table of Contents
- [☄ Overview](#-overview)
- [📖 Table of Contents](#-table-of-contents)
- [🚀 Features](#-features)
- [📦 Usage](#-usage)
- [🔄 Workflow](#-workflow)
- [📊 Benchmarks](#-benchmarks)
  - [Log literal strings (without formatting)](#log-literal-strings-without-formatting)
  - [Log strings with formatting](#log-strings-with-formatting)
- [🔧 Installation](#-installation)
  - [Using CMake](#using-cmake)
- [🔌 Custom Sinks](#-custom-sinks)
  - [✨ Implement Your Own Sink](#-implement-your-own-sink)
- [🪪 License](#-license)
- [❤️ Credits](#️-credits)


## 🚀 Features

- 🔧 Compile-time format string registration
- 🧵 True asynchronous logging pipeline
- 🎯 Zero dynamic memory allocations on the frontend
- 💾 Dedicated backend worker thread for formatting and output
- ⚡ Faster than `spdlog`, and `quill` in benchmark

---

## 📦 Usage

`femtolog` supports formatting messages using [fmtlib](https://github.com/fmtlib/fmt).

```cpp
#include "femtolog/femtolog.h"

int main() {
  // get thread local logger instance
  femtolog::Logger logger = femtolog::Logger::logger();

  // initialize logger and register log sink
  logger.init();
  logger.register_sink<femtolog::StdoutSink<>>();
  logger.register_sink<femtolog::FileSink<>>("");
  logger.level("trace");

  // start the backend worker that dequeues logged entries
  logger.start_worker();

  std::string username = "pugur";
  float cpu_usage = 42.57;
  bool result = true;
  int error_code = -1;

  // log messages with compile-time format strings:
  logger.trace<"Hello {}\n">("World");
  logger.debug<"Hello World wo formatting\n">();
  logger.info<"User \"{}\" logged in.\n">(username);
  logger.warn<"CPU usage is high: {}%\n">(cpu_usage);
  logger.error<"Return value is: {}\n">(result);

  logger.fatal<"Fatal error occured; error code: {}\n">(error_code);

  logger.stop_worker();
  logger.clear_sinks();

  return 0;
}
```

String literals (`"..."`) are registered at compile time. Arguments are serialized into a raw byte stream and passed through the pipeline asynchronously.

## 🔄 Workflow

The logging pipeline consists of a frontend (thread-local logger) and a backend (worker thread):
```
logger (frontend)
   |
   |-- serialize(format_id, format_args...) --> [ SPSC Queue ]
                                                      |
                                                      v
                                                  backend_worker (async)
                                                  |
                                                  |-- deserialize + formatting
                                                  |
                                                  '--> sink (stdout, files, custom target)
```
This architecture separates formatting from the hot path of logging, minimizing latency.

## 📊 Benchmarks

The following benchmark results were measured using [Google Benchmark](https://github.com/google/benchmark) performed on Clang-21 -O3 Release build, Intel Core i3 12100, 64GB 3600MHz RAM, Ubuntu 22.04 x86_64.
The benchmark codes are available in [`//src/bench/`](src/bench/) directory and the detail results of benchmark are archived in [`//src/bench/results/archive`](src/bench/results/archive/) directory.

### Log literal strings (without formatting)

| Library      | Median Latency (ns) | Throughput (msgs/sec) |
| :----------- | :------------------ | :-------------------- |
| **femtolog** | **3.17 ns**         | **\~218.4M**          |
| quill        | 14.8 ns             | \~47.2M               |
| spdlog       | 27.6 ns             | \~27.3M               |

### Log strings with formatting

| Library      | Median Latency (ns) | Throughput (msgs/sec) |
| :----------- | :------------------ | :-------------------- |
| **femtolog** | **9.67 ns**         | **\~69.1M**           |
| quill        | 14.8 ns             | \~46.9M               |
| spdlog       | 50.2 ns             | \~13.6M               |


## 🔧 Installation

### Using CMake

Add this repository as a git submodule:
```bash
git submodule add https://github.com/pugur523/femtolog.git ./femtolog --recursive
```

Add `femtolog` as a subdirectory:

```cmake
add_subdirectory(femtolog)

target_link_libraries(your_target PRIVATE femtolog)
```

To install the compiled library:
```cmake
set(INSTALL_FEMTOLOG TRUE)
set(FEMTOLOG_INSTALL_HEADERS TRUE)
add_subdirectory(femtolog)

target_link_libraries(your_target PRIVATE femtolog)
```

## 🔌 Custom Sinks
Need to log to a database, a network socket, or a ring buffer?

`femtolog` supports plug-and-play custom sinks via a simple interface:

### ✨ Implement Your Own Sink
To define a custom sink, just inherit from `SinkBase` and implement `on_log()`:
```cpp
#include "femtolog/sinks/sink_base.h"

class MySink : public femtolog::SinkBase {
 public:
  void on_log(const LogEntry& entry, const char* content, std::size_t len) override {
    // Write to file, send over network, etc.
    std::fwrite(content, 1, len, stderr);
  }
};
```
Then register your sink with the logger:
```cpp
logger.register_sink<MySink>();
```
That's it — your sink will now receive fully formatted log entries, asynchronously, from the backend.

## 🪪 License
`femtolog` is licensed under the [Apache 2.0 License](LICENSE).

## ❤️ Credits

- **[zlib](https://github.com/madler/zlib)**<br/>
  Used in `FileSink` and `JsonLinesSink` for compressing log files efficiently.
- **[GoogleTest (gtest)](https://github.com/google/googletest)**<br/>
  Serves as the primary unit testing framework for the entire project.
- **[Google Benchmark](https://github.com/google/benchmark)**<br/>
  Used to benchmark `femtolog` against other logging libraries to ensure high performance.
- **[fmtlib](https://github.com/fmtlib/fmt)**<br/>
  Powers the formatting engine behind all log message rendering.


Built with love by [pugur](https://github.com/pugur523).
