// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef CORE_CHECK_H_
#define CORE_CHECK_H_

#include <iostream>

#include "build/build_flag.h"
#include "core/base/core_export.h"
#include "core/base/string_util.h"

namespace core {

#define CHECK_IMPL(condition, type) \
  if (!(condition))                 \
  ::core::CheckFailureStream(#type, __FILE__, __LINE__, #condition).stream()

#define CHECK_W_OP_IMPL(val1, val2, op, type)                              \
  !(static_cast<bool>((val1)op(val2)))                                     \
      ? ::core::log_check_failure(#type, __FILE__, __LINE__,               \
                                  #val1 " " #op " " #val2, (val1), (val2)) \
      : ::core::null_stream()

#define CHECK(condition) CHECK_IMPL(condition, CHECK)
#define CHECK_EQ(val1, val2) CHECK_W_OP_IMPL(val1, val2, ==, CHECK)
#define CHECK_NE(val1, val2) CHECK_W_OP_IMPL(val1, val2, !=, CHECK)
#define CHECK_GT(val1, val2) CHECK_W_OP_IMPL(val1, val2, >, CHECK)
#define CHECK_GE(val1, val2) CHECK_W_OP_IMPL(val1, val2, >=, CHECK)
#define CHECK_LT(val1, val2) CHECK_W_OP_IMPL(val1, val2, <, CHECK)
#define CHECK_LE(val1, val2) CHECK_W_OP_IMPL(val1, val2, <=, CHECK)

#if IS_RELEASE
#define DCHECK(condition) \
  if constexpr (false)    \
  ::core::null_stream()
#define DCHECK_EQ(val1, val2) \
  if constexpr (false)        \
  ::core::null_stream()
#define DCHECK_NE(val1, val2) \
  if constexpr (false)        \
  ::core::null_stream()
#define DCHECK_GT(val1, val2) \
  if constexpr (false)        \
  ::core::null_stream()
#define DCHECK_GE(val1, val2) \
  if constexpr (false)        \
  ::core::null_stream()
#define DCHECK_LT(val1, val2) \
  if constexpr (false)        \
  ::core::null_stream()
#define DCHECK_LE(val1, val2) \
  if constexpr (false)        \
  ::core::null_stream()
#else
#define DCHECK(condition) CHECK_IMPL(condition, DCHECK)
#define DCHECK_EQ(val1, val2) CHECK_W_OP_IMPL(val1, val2, ==, DCHECK)
#define DCHECK_NE(val1, val2) CHECK_W_OP_IMPL(val1, val2, !=, DCHECK)
#define DCHECK_GT(val1, val2) CHECK_W_OP_IMPL(val1, val2, >, DCHECK)
#define DCHECK_GE(val1, val2) CHECK_W_OP_IMPL(val1, val2, >=, DCHECK)
#define DCHECK_LT(val1, val2) CHECK_W_OP_IMPL(val1, val2, <, DCHECK)
#define DCHECK_LE(val1, val2) CHECK_W_OP_IMPL(val1, val2, <=, DCHECK)
#endif  // IS_RELEASE

class CORE_EXPORT CheckFailureStream {
 public:
  CheckFailureStream(const char* type,
                     const char* file,
                     int line,
                     const char* condition);
  ~CheckFailureStream();
  std::ostream& stream();

 private:
  const char* type_;
  const char* file_;
  int line_;
  const char* condition_;
  bool has_output_ = false;
};

template <typename L, typename R>
std::ostream& log_check_failure(const char* type,
                                const char* file,
                                int line,
                                const char* expression,
                                const L& lhs,
                                const R& rhs) {
  auto& os = CheckFailureStream(type, file, line, expression).stream();

  if constexpr (is_ostreamable_v<R> && is_ostreamable_v<L>) {
    os << "  expected: " << rhs << "\n"
       << "    actual: " << lhs << "\n";
  } else {
    os << "  [value output not available for operands]\n";
  }

  return os;
}

class NullBuffer : public std::streambuf {
 public:
  int overflow(int c) override { return c; }
};

class NullStream : public std::ostream {
 public:
  NullStream() : std::ostream(&buffer_) {}

 private:
  NullBuffer buffer_;
};

inline std::ostream& null_stream() {
  static NullStream instance;
  return instance;
}

}  // namespace core

#endif  // CORE_CHECK_H_
