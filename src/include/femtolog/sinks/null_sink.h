// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_SINKS_NULL_SINK_H_
#define INCLUDE_FEMTOLOG_SINKS_NULL_SINK_H_

#include "femtolog/base/log_entry.h"
#include "femtolog/sinks/sink_base.h"

namespace femtolog {

class NullSink final : public SinkBase {
 public:
  NullSink() = default;
  ~NullSink() override = default;

  inline void on_log(const LogEntry&, const char*, std::size_t) override {
    // noop
  };
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_SINKS_NULL_SINK_H_
