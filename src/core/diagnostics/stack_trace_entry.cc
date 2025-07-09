// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/core/diagnostics/stack_trace_entry.h"

#include <cstddef>
#include <cstdio>

#include "femtolog/core/base/string_util.h"

namespace femtolog::core {

void StackTraceEntry::to_string(char* out_buf, std::size_t out_buf_size) const {
  char* cursor = out_buf;
  const char* end = out_buf + out_buf_size;

  if (use_index) {
    std::size_t index_len = write_format(cursor, end, "@{}", index);
    padding(cursor, end, index_len, kIndexAlignLength);
  }

  if (address[0]) {
    std::size_t addr_len = write_format(cursor, end, "{}", address.data());
    padding(cursor, end, addr_len, kAddressAlignLength);
  }

  const char* func = function[0] ? function.data() : kUnknownFunction;
  std::size_t func_len = write_format(cursor, end, "{}", func);

  if (offset > 0) {
    std::size_t offset_len = write_format(cursor, end, "+0x{}", offset);
    func_len += offset_len;
  }

  padding(cursor, end, func_len, kFunctionAlignLength);

  if (file[0]) {
    write_format(cursor, end, " at {}", file.data());
    if (line > 0) {
      write_format(cursor, end, ":{}", line);
    }
  }

  // Ensure null termination
  if (cursor < end) {
    *cursor = '\0';
  } else if (out_buf_size > 0) {
    out_buf[out_buf_size - 1] = '\0';
  }
}

}  // namespace femtolog::core
