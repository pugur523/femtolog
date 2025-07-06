// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/core/diagnostics/signal_handler.h"

#include <csignal>
#include <ctime>
#include <iostream>
#include <thread>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/diagnostics/stack_trace.h"

#if FEMTOLOG_IS_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace core {

namespace {

inline int get_pid() {
#if FEMTOLOG_IS_WINDOWS
  return static_cast<int>(GetCurrentProcessId());
#else
  return getpid();
#endif
}

}  // namespace

const char* signal_to_string(int signal_number) {
  switch (signal_number) {
    case SIGSEGV: return "SIGSEGV (Invalid access to storage)";
    case SIGABRT: return "SIGABRT (Abnormal termination)";
    case SIGFPE: return "SIGFPE (Floating point exception)";
    case SIGILL: return "SIGILL (Illegal instruction)";
    case SIGINT: return "SIGINT (Interactive attention signal)";
    case SIGTERM: return "SIGTERM (Termination request)";
    // case SIGBUS: return "SIGBUS (Bus error)";
    // case SIGKILL: return "SIGKILL (Kill signal)";
    // case SIGSTOP: return "SIGSTOP (Stop signal)";
    // case SIGALRM: return "SIGALRM (Alarm clock)";
    default: return "Unknown signal";
  }
}

// Example signal handling output:
//
// Aborted at Thu Jan  1 00:00:00 1970
// (1234567890 in unix time)
// SIGABRT (Aborted) received by PID 12345(TID 67890)
void signal_handler(int signal_number) {
  std::time_t now = std::time(nullptr);
  std::cout << "Aborted at " << std::ctime(&now) << "\n"
            << "(" << now << " in unix time)\n"
            << signal_to_string(signal_number) << " received by PID "
            << get_pid() << "(TID " << std::this_thread::get_id() << ")\n"
            << stack_trace_from_current_context() << "\n";

  std::exit(EXIT_FAILURE);
}

void register_signal_handlers() {
  std::signal(SIGSEGV, signal_handler);
  std::signal(SIGABRT, signal_handler);
  std::signal(SIGFPE, signal_handler);
  std::signal(SIGILL, signal_handler);

#if FEMTOLOG_IS_DEBUG
  std::signal(SIGINT, signal_handler);
#endif
}

}  // namespace core
