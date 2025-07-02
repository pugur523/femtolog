# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

if(FEMTOLOG_INSTALL_HEADERS)
  set(PUBLIC_HEADERS
    femtolog/femtolog.h
    femtolog/logger.h
    femtolog/base/format_string_registry.h
    femtolog/base/format_util.h
    femtolog/base/log_entry.h
    femtolog/base/log_level.h
    femtolog/sinks/file_sink.h
    femtolog/sinks/json_lines_sink.h
    femtolog/sinks/null_sink.h
    femtolog/sinks/sink_base.h
    femtolog/sinks/stdout_sink.h
  )
  install(
    FILES ${PUBLIC_HEADERS}
    DESTINATION include
    COMPONENT Header
  )
endif()
