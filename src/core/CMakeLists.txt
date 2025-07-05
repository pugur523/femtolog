# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set(MODULE_NAME "femtolog_core")
set(MODULE_OBJECTS_NAME "femtolog_core_objects")

message(STATUS "Configuring ${MODULE_NAME} module...")

set(SOURCES
  check.cc
  base/file_util.cc
  base/string_util.cc
)

add_library(${MODULE_OBJECTS_NAME} OBJECT ${SOURCES})

femtolog_setup_module(${MODULE_NAME} ${MODULE_OBJECTS_NAME}
  INCLUDE_DIRS ${FEMTOLOG_INCLUDE_DIRECTORIES}
  LINK_DIRS ${FEMTOLOG_LINK_DIRECTORIES}
  COMPILE_OPTIONS ${FEMTOLOG_COMPILE_OPTIONS}
  LINK_OPTIONS ${FEMTOLOG_LINK_OPTIONS}
  LINK_LIBS ${ZLIB_LIBRARIES} ${FEMTOLOG_LINK_LIBRARIES}
  COMPILE_DEFINITIONS ${FEMTOLOG_COMPILE_DEFINITIONS}
)

if(FEMTOLOG_ENABLE_VERBOSE)
  describe_target(${MODULE_NAME})
endif()
