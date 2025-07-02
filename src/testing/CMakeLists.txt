# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set(TEST_NAME ${MAIN_LIB_NAME}_test)

message(STATUS "Configuring ${TEST_NAME}...")

set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cc

  ${PROJECT_SOURCE_DIR}/femtolog/femtolog_test.cc

  ${PROJECT_SOURCE_DIR}/logging/impl/args_serializer_test.cc
  ${PROJECT_SOURCE_DIR}/logging/impl/spsc_queue_test.cc
  ${PROJECT_SOURCE_DIR}/logging/impl/internal_logger_test.cc
)

add_executable(${TEST_NAME} ${SOURCES})

target_include_directories(${TEST_NAME} PRIVATE ${FEMTOLOG_INCLUDE_DIRECTORIES} ${GTEST_INCLUDE_DIR} ${GMOCK_INCLUDE_DIR})
target_link_directories(${TEST_NAME} PRIVATE ${FEMTOLOG_LINK_DIRECTORIES})
target_link_libraries(${TEST_NAME} PRIVATE femtolog ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} ${FEMTOLOG_LINK_LIBRARIES})
target_compile_options(${TEST_NAME} PRIVATE ${PROJECT_COMPILE_OPTIONS})
target_link_options(${TEST_NAME} PRIVATE ${FEMTOLOG_LINK_OPTIONS})

set_target_properties(${TEST_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE TRUE
)

set(NEED_RUN TRUE)

if(MINGW_BUILD)
  set(EXE_COMMAND wine $<TARGET_FILE:${TEST_NAME}>)
else()
  set(EXE_COMMAND $<TARGET_FILE:${TEST_NAME}>)
endif()

if(FEMTOLOG_ENABLE_COVERAGE AND FEMTOLOG_ENABLE_RUN_TESTING_POST_BUILD)
  set(COVERAGE_DIR "${CMAKE_BINARY_DIR}/coverage/${TEST_NAME}")
  FILE(MAKE_DIRECTORY ${COVERAGE_DIR})
  add_custom_command(TARGET ${TEST_NAME} POST_BUILD
    COMMENT "Run & generate coverage report"
    COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${COVERAGE_DIR}/profile.profraw ${EXE_COMMAND}
    COMMAND llvm-profdata merge -output=${COVERAGE_DIR}/merged.profdata ${COVERAGE_DIR}/profile.profraw
    COMMAND llvm-cov show $<TARGET_FILE:${TEST_NAME}>
    -instr-profile=${COVERAGE_DIR}/merged.profdata
    -format=html
    -output-dir=${COVERAGE_DIR}/html

    WORKING_DIRECTORY ${PROJECT_ROOT_DIR}
  )
  set(NEED_RUN FALSE)
endif()

# if(FEMTOLOG_ENABLE_XRAY AND FEMTOLOG_ENABLE_RUN_TESTING_POST_BUILD)
# set(XRAY_OPTIONS "xray_mode=xray-basic:verbosity=1:patch_premain=true")
# set(XRAY_DIR ${CMAKE_BINARY_DIR}/xray/${TEST_NAME})
# FILE(MAKE_DIRECTORY ${XRAY_DIR})
# set(EXE_PATH $<TARGET_FILE:${TEST_NAME}>)
# add_custom_command(TARGET ${TEST_NAME} POST_BUILD
# COMMENT "Run xray instrumentation and generate function timing report"
# COMMAND rm -f ${XRAY_DIR}/xray-log.${TEST_NAME}.*
# COMMAND ${CMAKE_COMMAND} -E env XRAY_OPTIONS="${XRAY_OPTIONS}" ${CMAKE_COMMAND} -E time ${EXE_PATH}
#
# COMMAND llvm-xray convert --instr_map=${EXE_PATH} --sort --output-format=yaml --symbolize --demangle --output=${XRAY_DIR}/xray-report.yml ${XRAY_DIR}/xray-log.${TEST_NAME}.*
# COMMAND llvm-xray convert --instr_map=${EXE_PATH} --sort --output-format=trace_event --symbolize --demangle --output=${XRAY_DIR}/xray_trace.${TEST_NAME}.json ${XRAY_DIR}/xray-log.${TEST_NAME}.*
# COMMAND llvm-xray account --instr_map=${EXE_PATH} --sort=count --top=50 --format=text --output=${XRAY_DIR}/xray-summary.txt ${XRAY_DIR}/xray-log.${TEST_NAME}.*
#
# WORKING_DIRECTORY ${XRAY_DIR}
# )
# set(NEED_RUN FALSE)
# endif()
if(NEED_RUN AND FEMTOLOG_ENABLE_RUN_TESTING_POST_BUILD)
  add_custom_command(TARGET ${TEST_NAME} POST_BUILD
    COMMENT "Run tests if it has never been executed."
    COMMAND ${EXE_COMMAND}
  )
endif()

if(FEMTOLOG_INSTALL_TESTING)
  install(
    TARGETS ${TEST_NAME}
    RUNTIME
    COMPONENT Runtime
  )

  install(
    FILES "${CMAKE_PDB_OUTPUT_DIRECTORY}/${TEST_NAME}.pdb"
    DESTINATION bin
    COMPONENT Runtime
    OPTIONAL
  )
endif()

add_test(
  NAME ${TEST_NAME}
  COMMAND ${TEST_NAME}
)

if(FEMTOLOG_ENABLE_VERBOSE)
  describe_target(${TEST_NAME})
endif()

# include(GoogleTest)
# gtest_discover_tests(${TEST_NAME})
