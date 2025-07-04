// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BUILD_COMPONENT_EXPORT_H_
#define INCLUDE_FEMTOLOG_BUILD_COMPONENT_EXPORT_H_

// defines FEMTOLOG_IS_WINDOWS, FEMTOLOG_COMPILER_CLANG, etc.
#include "femtolog/build/build_flag.h"

// Platform-specific attributes
#if FEMTOLOG_IS_WINDOWS
#define FEMTOLOG_COMPONENT_EXPORT_ANNOTATION __declspec(dllexport)
#define FEMTOLOG_COMPONENT_IMPORT_ANNOTATION __declspec(dllimport)
#define FEMTOLOG_COMPONENT_HIDDEN_ANNOTATION
#elif FEMTOLOG_COMPILER_GCC || FEMTOLOG_COMPILER_CLANG
#define FEMTOLOG_COMPONENT_EXPORT_ANNOTATION \
  __attribute__((visibility("default")))
#define FEMTOLOG_COMPONENT_IMPORT_ANNOTATION \
  __attribute__((visibility("default")))
#define FEMTOLOG_COMPONENT_HIDDEN_ANNOTATION \
  __attribute__((visibility("hidden")))
#else
#define FEMTOLOG_COMPONENT_EXPORT_ANNOTATION
#define FEMTOLOG_COMPONENT_IMPORT_ANNOTATION
#define FEMTOLOG_COMPONENT_HIDDEN_ANNOTATION
#endif

// Conditional macro that selects between two values based on condition
#define FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_(condition, consequent,      \
                                              alternate)                  \
  FEMTOLOG_COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_(                        \
      FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_COMMA_(condition), consequent, \
      alternate)

// Generates a comma if condition is 1, nothing if condition is 0 or undefined
#define FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_COMMA_(...) \
  FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_COMMA_IMPL_(__VA_ARGS__, 0)

#define FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_COMMA_IMPL_(x, ...) \
  FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_COMMA_##x##_

#define FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_COMMA_1_ ,
#define FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_COMMA_0_
#define FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_COMMA__

// Selects the third argument (used with comma trick)
#define FEMTOLOG_COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_(...) \
  FEMTOLOG_COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_IMPL_(__VA_ARGS__, 0, 0)

#define FEMTOLOG_COMPONENT_MACRO_SELECT_THIRD_ARGUMENT_IMPL_(a, b, c, ...) c

#define FEMTOLOG_COMPONENT_EXPORT(component)                                \
  FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_(                                    \
      FEMTOLOG_IS_##component##_STATIC, /* static build - no annotation */, \
      FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_(                                \
          FEMTOLOG_IS_##component##_IMPL,                                   \
          FEMTOLOG_COMPONENT_EXPORT_ANNOTATION,                             \
          FEMTOLOG_COMPONENT_IMPORT_ANNOTATION))

// Utility macro to check if we're inside the component implementation
#define FEMTOLOG_INSIDE_COMPONENT_IMPL(component) \
  FEMTOLOG_COMPONENT_MACRO_CONDITIONAL_(FEMTOLOG_IS_##component##_IMPL, 1, 0)

// Shortcut for hidden visibility
#define FEMTOLOG_COMPONENT_HIDDEN FEMTOLOG_COMPONENT_HIDDEN_ANNOTATION

// Convenience macro for generating component-specific export macros
#define FEMTOLOG_DECLARE_COMPONENT_EXPORT(component) \
  static_assert(true); /* force semicolon */         \
  constexpr auto component##_EXPORT = FEMTOLOG_COMPONENT_EXPORT(component)

#endif  // INCLUDE_FEMTOLOG_BUILD_COMPONENT_EXPORT_H_
