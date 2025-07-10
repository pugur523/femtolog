# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set(BENCHMARK_NAME ${MAIN_LIB_NAME}_bench)

message(STATUS "Configuring ${BENCHMARK_NAME}...")

set(SOURCES
  bench_main.cc

  femtolog_bench.cc
)

set(FEMTOLOG_INTERNAL_BENCH_SOURCES
  bench_main.cc

  # format_bench.cc

  # ${PROJECT_SOURCE_DIR}/core/base/file_util_bench.cc
  # ${PROJECT_SOURCE_DIR}/core/base/string_util_bench.cc
  ${PROJECT_SOURCE_DIR}/logging/impl/args_deserializer_bench.cc
  ${PROJECT_SOURCE_DIR}/logging/impl/args_serializer_bench.cc
  ${PROJECT_SOURCE_DIR}/logging/impl/backend_worker_bench.cc
  ${PROJECT_SOURCE_DIR}/logging/impl/internal_logger_bench.cc
  ${PROJECT_SOURCE_DIR}/logging/impl/spmc_queue_bench.cc
  ${PROJECT_SOURCE_DIR}/logging/impl/spsc_queue_bench.cc
)

include(FetchContent)

if(FEMTOLOG_ENABLE_BENCH_QUILL)
  FetchContent_Declare(
    quill
    GIT_REPOSITORY https://github.com/odygrd/quill.git
    GIT_TAG v10.0.0
  )
  set(QUILL_NO_EXCEPTIONS TRUE CACHE BOOL "" FORCE)
  set(QUILL_DISABLE_NON_PREFIXED_MACROS TRUE CACHE BOOL "" FORCE)
  set(QUILL_DISABLE_FUNCTION_NAME TRUE CACHE BOOL "" FORCE)
  # set(QUILL_DISABLE_FILE_INFO TRUE CACHE BOOL "" FORCE)

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
  set(SPDLOG_FMT_EXTERNAL TRUE CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(spdlog)
  target_compile_definitions(spdlog PUBLIC SPDLOG_NO_EXCEPTIONS=1)
  list(APPEND BENCHMARK_LIBRARIES spdlog)
  list(APPEND SOURCES spdlog_bench.cc)
endif()

add_executable(${BENCHMARK_NAME} ${SOURCES})
add_executable(${BENCHMARK_NAME}_internal ${FEMTOLOG_INTERNAL_BENCH_SOURCES})

target_include_directories(${BENCHMARK_NAME} PRIVATE ${FEMTOLOG_INCLUDE_DIRECTORIES})
target_compile_options(${BENCHMARK_NAME} PRIVATE ${FEMTOLOG_COMPILE_OPTIONS})
target_compile_definitions(${BENCHMARK_NAME} PRIVATE ${FEMTOLOG_COMPILE_DEFINITIONS})
target_link_options(${BENCHMARK_NAME} PRIVATE ${FEMTOLOG_LINK_OPTIONS})
target_link_directories(${BENCHMARK_NAME} PRIVATE ${FEMTOLOG_LINK_DIRECTORIES})
target_link_libraries(${BENCHMARK_NAME} PRIVATE femtolog ${BENCHMARK_LIBRARIES} ${GOOGLE_BENCHMARK_LIBRARIES} ${FEMTOLOG_LINK_LIBRARIES})

set_target_properties(${BENCHMARK_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE TRUE
)

target_include_directories(${BENCHMARK_NAME}_internal PRIVATE ${FEMTOLOG_INCLUDE_DIRECTORIES})
target_compile_options(${BENCHMARK_NAME}_internal PRIVATE ${FEMTOLOG_COMPILE_OPTIONS})
target_compile_definitions(${BENCHMARK_NAME}_internal PRIVATE ${FEMTOLOG_COMPILE_DEFINITIONS})
target_link_options(${BENCHMARK_NAME}_internal PRIVATE ${FEMTOLOG_LINK_OPTIONS})
target_link_directories(${BENCHMARK_NAME}_internal PRIVATE ${FEMTOLOG_LINK_DIRECTORIES})
target_link_libraries(${BENCHMARK_NAME}_internal PRIVATE femtolog ${BENCHMARK_LIBRARIES} ${GOOGLE_BENCHMARK_LIBRARIES} ${FEMTOLOG_LINK_LIBRARIES})

set_target_properties(${BENCHMARK_NAME}_internal PROPERTIES
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

  install(
    TARGETS ${BENCHMARK_NAME}_internal
    RUNTIME
    COMPONENT Runtime
  )

  install(
    FILES "${CMAKE_PDB_OUTPUT_DIRECTORY}/${BENCHMARK_NAME}_internal.pdb"
    DESTINATION bin
    COMPONENT Runtime
    OPTIONAL
  )
endif()

if(FEMTOLOG_ENABLE_VERBOSE)
  describe_target(${BENCHMARK_NAME})
  describe_target(${BENCHMARK_NAME}_internal)
endif()
