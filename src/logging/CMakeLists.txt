# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set(MODULE_NAME "femtolog_logging")
set(MODULE_OBJECTS_NAME "femtolog_logging_objects")

message(STATUS "Configuring ${MODULE_NAME} module...")

set(SOURCES
  impl/backend_worker.cc
  impl/internal_logger.cc
  impl/spsc_queue.cc
)

add_library(${MODULE_OBJECTS_NAME} OBJECT ${SOURCES})

femtolog_setup_module(${MODULE_NAME} ${MODULE_OBJECTS_NAME}
  INCLUDE_DIRS ${FEMTOLOG_INCLUDE_DIRECTORIES}
  LINK_DIRS ${FEMTOLOG_LINK_DIRECTORIES}
  COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS}
  LINK_OPTIONS ${FEMTOLOG_LINK_OPTIONS}
  LINK_LIBS femtolog_core ${FMTLIB_LIBRARIES} ${FEMTOLOG_LINK_LIBRARIES}
)

if(FEMTOLOG_ENABLE_VERBOSE)
  describe_target(${MODULE_NAME})
endif()
