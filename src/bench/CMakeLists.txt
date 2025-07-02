# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set(BENCHMARK_NAME ${MAIN_LIB_NAME}_bench)

message(STATUS "Configuring ${BENCHMARK_NAME}...")

set(SOURCES
  bench_main.cc

  ${PROJECT_SOURCE_DIR}/femtolog/femtolog_bench.cc

  # format_bench.cc

  # ${PROJECT_SOURCE_DIR}/core/base/file_util_bench.cc
  # ${PROJECT_SOURCE_DIR}/core/base/string_util_bench.cc
  # ${PROJECT_SOURCE_DIR}/logging/impl/args_serializer_bench.cc
  # ${PROJECT_SOURCE_DIR}/logging/impl/internal_logger_bench.cc
  # ${PROJECT_SOURCE_DIR}/logging/impl/log_entry_bench.cc
  # ${PROJECT_SOURCE_DIR}/logging/impl/spsc_queue_bench.cc
)

include(FetchContent)

if(FEMTOLOG_ENABLE_BENCH_QUILL)
  FetchContent_Declare(
    quill
    GIT_REPOSITORY https://github.com/odygrd/quill.git
    GIT_TAG v10.0.0
  )
  set(QUILL_NO_EXCEPTIONS TRUE)

  # set(QUILL_X86ARCH TRUE)
  FetchContent_MakeAvailable(quill)
  list(APPEND BENCHMARK_LIBRARIES quill::quill)
  list(APPEND SOURCES quill_bench.cc)
endif()

if(FEMTOLOG_ENABLE_BENCH_SPDLOG)
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
  )
  FetchContent_MakeAvailable(spdlog)
  target_compile_definitions(spdlog PUBLIC SPDLOG_NO_EXCEPTIONS)
  list(APPEND BENCHMARK_LIBRARIES spdlog)
  list(APPEND SOURCES spdlog_bench.cc)
endif()

add_executable(${BENCHMARK_NAME} ${SOURCES})

target_include_directories(${BENCHMARK_NAME} PRIVATE ${FEMTOLOG_INCLUDE_DIRECTORIES})
target_link_directories(${BENCHMARK_NAME} PRIVATE ${FEMTOLOG_LINK_DIRECTORIES})
target_link_libraries(${BENCHMARK_NAME} PRIVATE femtolog ${BENCHMARK_LIBRARIES} ${GOOGLE_BENCHMARK_LIBRARIES} ${FEMTOLOG_LINK_LIBRARIES})
target_compile_options(${BENCHMARK_NAME} PRIVATE ${PROJECT_COMPILE_OPTIONS})
target_link_options(${BENCHMARK_NAME} PRIVATE ${FEMTOLOG_LINK_OPTIONS})

set_target_properties(${BENCHMARK_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE TRUE
)

set(NEED_RUN TRUE)

if(MINGW_BUILD)
  set(EXE_COMMAND wine $<TARGET_FILE:${BENCHMARK_NAME}>)
else()
  set(EXE_COMMAND $<TARGET_FILE:${BENCHMARK_NAME}>)
endif()

if(FEMTOLOG_ENABLE_COVERAGE AND FEMTOLOG_ENABLE_RUN_BENCHMARK_POST_BUILD)
  set(COVERAGE_DIR "${CMAKE_BINARY_DIR}/coverage/${BENCHMARK_NAME}")
  FILE(MAKE_DIRECTORY ${COVERAGE_DIR})
  add_custom_command(TARGET ${BENCHMARK_NAME} POST_BUILD
    COMMENT "Run & generate coverage report"
    COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${COVERAGE_DIR}/profile.profraw ${EXE_COMMAND}
    COMMAND llvm-profdata merge -output=${COVERAGE_DIR}/merged.profdata ${COVERAGE_DIR}/profile.profraw
    COMMAND llvm-cov show $<TARGET_FILE:${BENCHMARK_NAME}>
    -instr-profile=${COVERAGE_DIR}/merged.profdata
    -format=html
    -output-dir=${COVERAGE_DIR}/html

    WORKING_DIRECTORY ${PROJECT_ROOT_DIR}
  )
  set(NEED_RUN FALSE)
endif()

# if(FEMTOLOG_ENABLE_XRAY AND FEMTOLOG_ENABLE_RUN_BENCHMARK_POST_BUILD)
# set(XRAY_OPTIONS "xray_mode=xray-basic:verbosity=1:patch_premain=true")
# set(XRAY_DIR ${CMAKE_BINARY_DIR}/xray/${BENCHMARK_NAME})
# FILE(MAKE_DIRECTORY ${XRAY_DIR})
# set(EXE_PATH $<TARGET_FILE:${BENCHMARK_NAME}>)
# add_custom_command(TARGET ${BENCHMARK_NAME} POST_BUILD
# COMMENT "Run xray instrumentation and generate function timing report"
# COMMAND rm -f ${XRAY_DIR}/xray-log.${BENCHMARK_NAME}.*
# COMMAND ${CMAKE_COMMAND} -E env XRAY_OPTIONS="${XRAY_OPTIONS}" ${CMAKE_COMMAND} -E time ${EXE_PATH}
#
# # set(XRAY_FILE ${XRAY_DIR}/xray-log.${BENCHMARK_NAME}.*)
# # COMMAND bash -c "
# # XRAY_FILE=$$(ls -t ${XRAY_DIR}/xray-log.${BENCHMARK_NAME}.* | head -n1) && \\
# # llvm-xray convert --instr_map='${EXE_PATH}' --sort --output-format=yaml --symbolize --demangle --output='${XRAY_DIR}/xray-report.yml' \"\${XRAY_FILE}\" && \\
# # llvm-xray convert --instr_map='${EXE_PATH}' --sort --output-format=trace_event --symbolize --demangle --output='${XRAY_DIR}/xray_trace.${BENCHMARK_NAME}.json' \"\${XRAY_FILE}\" && \\
# # llvm-xray account --instr_map='${EXE_PATH}' --sort=count --top=50 --format=text --output=${XRAY_DIR}/xray-summary.txt  \"\${XRAY_FILE}\"
# # "
# # COMMAND bash -c "XRAY_FILE=$$(ls -t ${XRAY_DIR}/xray-log.${BENCHMARK_NAME}.* | head -n1) && llvm-xray convert --instr_map='${EXE_PATH}' --sort --output-format=yaml --symbolize --demangle --output='${XRAY_DIR}/xray-report.yml' \"\${XRAY_FILE}\" && llvm-xray convert --instr_map='${EXE_PATH}' --sort --output-format=trace_event --symbolize --demangle --output='${XRAY_DIR}/xray_trace.${BENCHMARK_NAME}.json' \"\${XRAY_FILE}\" && llvm-xray account --instr_map=${EXE_PATH} --sort=count --top=50 --format=text --output=${XRAY_DIR}/xray-summary.txt  \"\${XRAY_FILE}\""
# WORKING_DIRECTORY ${XRAY_DIR}
# )
# set(NEED_RUN FALSE)
# endif()
if(NEED_RUN AND FEMTOLOG_ENABLE_RUN_BENCHMARK_POST_BUILD)
  add_custom_command(TARGET ${BENCHMARK_NAME} POST_BUILD
    COMMENT "Run benchmarks if it has never been executed."
    COMMAND ${EXE_COMMAND}
  )
endif()

if(FEMTOLOG_INSTALL_BENCHMARK)
  install(
    TARGETS ${BENCHMARK_NAME}
    RUNTIME
    COMPONENT Runtime
  )

  install(
    FILES "${CMAKE_PDB_OUTPUT_DIRECTORY}/${BENCHMARK_NAME}.pdb"
    DESTINATION bin
    COMPONENT Runtime
    OPTIONAL
  )
endif()

if(FEMTOLOG_ENABLE_VERBOSE)
  describe_target(${BENCHMARK_NAME})
endif()
