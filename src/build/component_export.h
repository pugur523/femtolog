// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef BUILD_COMPONENT_EXPORT_H_
#define BUILD_COMPONENT_EXPORT_H_

// defines IS_WINDOWS, COMPILER_CLANG, etc.
#include "build/build_flag.h"

// Platform-specific attributes
#if IS_WINDOWS
#define COMPONENT_EXPORT_ANNOTATION __declspec(dllexport)
#define COMPONENT_IMPORT_ANNOTATION __declspec(dllimport)
#define COMPONENT_HIDDEN_ANNOTATION
#elif COMPILER_GCC || COMPILER_CLANG
#define COMPONENT_EXPORT_ANNOTATION __attribute__((visibility("default")))
#define COMPONENT_IMPORT_ANNOTATION __attribute__((visibility("default")))
#define COMPONENT_HIDDEN_ANNOTATION __attribute__((visibility("hidden")))
#else
#define COMPONENT_EXPORT_ANNOTATION
#define COMPONENT_IMPORT_ANNOTATION
#define COMPONENT_HIDDEN_ANNOTATION
#endif

// Conditional macro that selects between two values based on condition
#define COMPONENT_MACRO_CONDITIONAL_(condition, consequent, alternate) \
  COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_(                              \
      COMPONENT_MACRO_CONDITIONAL_COMMA_(condition), consequent, alternate)

// Generates a comma if condition is 1, nothing if condition is 0 or undefined
#define COMPONENT_MACRO_CONDITIONAL_COMMA_(...) \
  COMPONENT_MACRO_CONDITIONAL_COMMA_IMPL_(__VA_ARGS__, 0)

#define COMPONENT_MACRO_CONDITIONAL_COMMA_IMPL_(x, ...) \
  COMPONENT_MACRO_CONDITIONAL_COMMA_##x##_

#define COMPONENT_MACRO_CONDITIONAL_COMMA_1_ ,
#define COMPONENT_MACRO_CONDITIONAL_COMMA_0_
#define COMPONENT_MACRO_CONDITIONAL_COMMA__

// Selects the third argument (used with comma trick)
#define COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_(...) \
  COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_IMPL_(__VA_ARGS__, 0, 0)

#define COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_IMPL_(a, b, c, ...) c

#define COMPONENT_EXPORT(component)                                \
  COMPONENT_MACRO_CONDITIONAL_(                                    \
      IS_##component##_STATIC, /* static build - no annotation */, \
      COMPONENT_MACRO_CONDITIONAL_(IS_##component##_IMPL,          \
                                   COMPONENT_EXPORT_ANNOTATION,    \
                                   COMPONENT_IMPORT_ANNOTATION))

// Utility macro to check if we're inside the component implementation
#define INSIDE_COMPONENT_IMPL(component) \
  COMPONENT_MACRO_CONDITIONAL_(IS_##component##_IMPL, 1, 0)

// Shortcut for hidden visibility
#define COMPONENT_HIDDEN COMPONENT_HIDDEN_ANNOTATION

// Convenience macro for generating component-specific export macros
#define DECLARE_COMPONENT_EXPORT(component)  \
  static_assert(true); /* force semicolon */ \
  constexpr auto component##_EXPORT = COMPONENT_EXPORT(component)

#endif  // BUILD_COMPONENT_EXPORT_H_
