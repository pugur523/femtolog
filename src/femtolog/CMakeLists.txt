# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set(MODULE_NAME ${MAIN_LIB_NAME})
set(MODULE_OBJECTS_NAME "${MODULE_NAME}_objects")

message(STATUS "Configuring ${MODULE_NAME} module...")

set(SOURCES
  femtolog.cc
)

add_library(${MODULE_OBJECTS_NAME} OBJECT ${SOURCES})

femtolog_setup_module(${MODULE_NAME} ${MODULE_OBJECTS_NAME}
  INCLUDE_DIRS ${FEMTOLOG_INCLUDE_DIRECTORIES}
  COMPILE_OPTIONS ${FEMTOLOG_COMPILE_OPTIONS}
  COMPILE_DEFINITIONS ${FEMTOLOG_COMPILE_DEFINITIONS}
  LINK_OPTIONS ${FEMTOLOG_LINK_OPTIONS}
  LINK_DIRS ${FEMTOLOG_LINK_DIRECTORIES}
  LINK_LIBS femtolog_core femtolog_logging ${FMTLIB_LIBRARIES} ${FEMTOLOG_LINK_LIBRARIES}
  LINK_LIBS ${FEMTOLOG_LINK_LIBRARIES}
)

target_include_directories(${MODULE_NAME}
  PUBLIC 
    $<BUILD_INTERFACE:${FEMTOLOG_PUBLIC_INCLUDE_DIRECTORIES}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${MODULE_NAME} PUBLIC femtolog_core femtolog_logging ${FMTLIB_LIBRARIES})

set(COMPILE_COMMANDS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/../../..")
set(COMPILE_COMMANDS_FILE "${CMAKE_BINARY_DIR}/compile_commands.json")

add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${COMPILE_COMMANDS_FILE}"
  "${COMPILE_COMMANDS_OUTPUT_DIR}/compile_commands.json"

  WORKING_DIRECTORY ${PROJECT_ROOT_DIR}
)

if(FEMTOLOG_ENABLE_VERBOSE)
  describe_target(${MODULE_NAME})
endif()

if(FEMTOLOG_INSTALL_LIBS)
  install(
    TARGETS ${MODULE_NAME}
    RUNTIME
    COMPONENT Runtime
    ARCHIVE
    COMPONENT Archive
    LIBRARY
    COMPONENT Library
  )

  install(
    FILES "$<TARGET_FILE_DIR:${MODULE_NAME}>/${MODULE_NAME}.pdb" "$<TARGET_FILE_DIR:${MODULE_NAME}>/${MODULE_NAME}d.pdb"
    DESTINATION bin
    COMPONENT Runtime
    OPTIONAL
  )
endif()

if(FEMTOLOG_INSTALL_HEADERS)
  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/
    DESTINATION include
    COMPONENT Headers
  )

  # install(
  #   DIRECTORY ${FEMTOLOG_INCLUDE_DIRECTORIES}/
  #   DESTINATION include
  #   COMPONENT Headers
  # )
endif()
