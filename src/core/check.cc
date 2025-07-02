// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "core/check.h"

#include <exception>
#include <iostream>

namespace core {

CheckFailureStream::CheckFailureStream(const char* type,
                                       const char* file,
                                       int line,
                                       const char* condition)
    : type_(type), file_(file), line_(line), condition_(condition) {}

std::ostream& CheckFailureStream::stream() {
  has_output_ = true;
  std::cerr << type_ << " failed: \"" << condition_ << "\" at " << file_ << ":"
            << line_ << "\n";
  return std::cerr;
}

CheckFailureStream::~CheckFailureStream() {
  if (has_output_) {
    std::cerr << std::flush;
  }
  std::terminate();
}

}  // namespace core
