// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_CORE_CHECK_H_
#define INCLUDE_FEMTOLOG_CORE_CHECK_H_

#include <iostream>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/core_export.h"
#include "femtolog/core/base/string_util.h"

namespace core {

#define FEMTOLOG_CHECK_IMPL(condition, type) \
  if (!(condition)) [[unlikely]]             \
  ::core::CheckFailureStream(#type, __FILE__, __LINE__, #condition).stream()

#define FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, op, type)                     \
  __builtin_expect(!(static_cast<bool>((val1)op(val2))), 0)                \
      ? ::core::log_check_failure(#type, __FILE__, __LINE__,               \
                                  #val1 " " #op " " #val2, (val1), (val2)) \
      : ::core::null_stream()

#define FEMTOLOG_CHECK(condition) FEMTOLOG_CHECK_IMPL(condition, FEMTOLOG_CHECK)
#define FEMTOLOG_CHECK_EQ(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, ==, FEMTOLOG_CHECK)
#define FEMTOLOG_CHECK_NE(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, !=, FEMTOLOG_CHECK)
#define FEMTOLOG_CHECK_GT(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, >, FEMTOLOG_CHECK)
#define FEMTOLOG_CHECK_GE(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, >=, FEMTOLOG_CHECK)
#define FEMTOLOG_CHECK_LT(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, <, FEMTOLOG_CHECK)
#define FEMTOLOG_CHECK_LE(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, <=, FEMTOLOG_CHECK)

#if FEMTOLOG_IS_RELEASE
#define FEMTOLOG_DCHECK(condition) \
  if constexpr (false)             \
  ::core::null_stream()
#define FEMTOLOG_DCHECK_EQ(val1, val2) \
  if constexpr (false)                 \
  ::core::null_stream()
#define FEMTOLOG_DCHECK_NE(val1, val2) \
  if constexpr (false)                 \
  ::core::null_stream()
#define FEMTOLOG_DCHECK_GT(val1, val2) \
  if constexpr (false)                 \
  ::core::null_stream()
#define FEMTOLOG_DCHECK_GE(val1, val2) \
  if constexpr (false)                 \
  ::core::null_stream()
#define FEMTOLOG_DCHECK_LT(val1, val2) \
  if constexpr (false)                 \
  ::core::null_stream()
#define FEMTOLOG_DCHECK_LE(val1, val2) \
  if constexpr (false)                 \
  ::core::null_stream()
#else
#define FEMTOLOG_DCHECK(condition) \
  FEMTOLOG_CHECK_IMPL(condition, FEMTOLOG_DCHECK)
#define FEMTOLOG_DCHECK_EQ(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, ==, FEMTOLOG_DCHECK)
#define FEMTOLOG_DCHECK_NE(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, !=, FEMTOLOG_DCHECK)
#define FEMTOLOG_DCHECK_GT(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, >, FEMTOLOG_DCHECK)
#define FEMTOLOG_DCHECK_GE(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, >=, FEMTOLOG_DCHECK)
#define FEMTOLOG_DCHECK_LT(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, <, FEMTOLOG_DCHECK)
#define FEMTOLOG_DCHECK_LE(val1, val2) \
  FEMTOLOG_CHECK_W_OP_IMPL(val1, val2, <=, FEMTOLOG_DCHECK)
#endif  // FEMTOLOG_IS_RELEASE

class FEMTOLOG_CORE_EXPORT CheckFailureStream {
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

#endif  // INCLUDE_FEMTOLOG_CORE_CHECK_H_
