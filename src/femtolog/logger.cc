// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logger.h"

namespace femtolog {

// staic
Logger& Logger::logger() {
  thread_local Logger logger_;
  return logger_;
}

// static
Logger& Logger::global_logger() {
  static Logger logger_;
  return logger_;
}

}  // namespace femtolog
