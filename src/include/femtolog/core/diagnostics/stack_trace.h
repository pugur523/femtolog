// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_CORE_DIAGNOSTICS_STACK_TRACE_H_
#define INCLUDE_FEMTOLOG_CORE_DIAGNOSTICS_STACK_TRACE_H_

#include <cstddef>
#include <string>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/core_export.h"

namespace femtolog::core {

struct StackTraceEntry;

static constexpr std::size_t kDefaultFirstFrame = 1;

#if FEMTOLOG_IS_WINDOWS
static constexpr std::size_t kPlatformMaxFrames = 62;
#elif FEMTOLOG_IS_MAC
static constexpr std::size_t kPlatformMaxFrames = 128;
#elif FEMTOLOG_IS_LINUX
static constexpr std::size_t kPlatformMaxFrames = 64;
#else
static constexpr std::size_t kPlatformMaxFrames = 64;
#endif

constexpr const std::size_t kLineBufferSize = 1024;
constexpr const std::size_t kSymbolBufferSize = 512;
constexpr const std::size_t kDemangledBufferSize = 512;
constexpr const std::size_t kMangledBufferSize = 256;

// Example stack trace output:
//
// @0     0x123456789abc   bar()        at /path/to/file
// @1     0x23456789abcd   foo()        at /path/to/file
// @2     0x3456789abcde   hoge()       at /path/to/file
// @3     0x456789abcdef   fuga()       at /path/to/file

[[nodiscard]] FEMTOLOG_CORE_EXPORT std::string stack_trace_from_current_context(
    bool use_index = true,
    std::size_t first_frame = kDefaultFirstFrame,
    std::size_t max_frames = kPlatformMaxFrames);

FEMTOLOG_CORE_EXPORT void stack_trace_from_current_context(
    char* buffer,
    std::size_t buffer_size,
    bool use_index = true,
    std::size_t first_frame = kDefaultFirstFrame,
    std::size_t max_frames = kPlatformMaxFrames);

FEMTOLOG_CORE_EXPORT void register_stack_trace_handler();

}  // namespace femtolog::core

#endif  // INCLUDE_FEMTOLOG_CORE_DIAGNOSTICS_STACK_TRACE_H_
