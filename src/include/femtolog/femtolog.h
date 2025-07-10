// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_FEMTOLOG_H_
#define INCLUDE_FEMTOLOG_FEMTOLOG_H_

#include "femtolog/base/femtolog_export.h"
#include "femtolog/base/format_util.h"
#include "femtolog/base/log_entry.h"
#include "femtolog/base/log_level.h"
#include "femtolog/base/string_registry.h"
#include "femtolog/core/diagnostics/signal_handler.h"
#include "femtolog/core/diagnostics/stack_trace.h"
#include "femtolog/core/diagnostics/terminate_handler.h"
#include "femtolog/logger.h"
#include "femtolog/options.h"
#include "femtolog/sinks/file_sink.h"
#include "femtolog/sinks/json_lines_sink.h"
#include "femtolog/sinks/null_sink.h"
#include "femtolog/sinks/sink_base.h"
#include "femtolog/sinks/stdout_sink.h"

namespace femtolog {

inline void register_signal_handlers() {
  core::register_signal_handlers();
}

inline void register_terminate_handler() {
  core::register_terminate_handler();
}

inline void register_stack_trace_handler() {
  core::register_stack_trace_handler();
}

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_FEMTOLOG_H_
