# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set(CMAKE_C_STANDARD ${PROJECT_C_VERSION})
set(CMAKE_CXX_STANDARD ${PROJECT_CXX_VERSION})
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROJECT_ROOT_DIR "${PROJECT_SOURCE_DIR}/..")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/build/cmake")
set(BUILD_RESOURCES_DIR "${PROJECT_SOURCE_DIR}/build/resources")
set(THIRD_PARTY_DIR "${PROJECT_SOURCE_DIR}/third_party")
set(THIRD_PARTY_INSTALL_DIR "${THIRD_PARTY_DIR}/install")
set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib")
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}:${CMAKE_BINARY_DIR}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_DEBUG_POSTFIX "d")

set(MINGW_BUILD FALSE)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(MINGW_BUILD TRUE)
endif()

option(DO_CLANG_TIDY "do clang tidy for linting" TRUE)

if(DO_CLANG_TIDY AND NOT MINGW_BUILD)
  set(CLANG_TIDY_COMMAND "clang-tidy;--config-file=${PROJECT_ROOT_DIR}/.clang-tidy;-p=${CMAKE_BINARY_DIR}")
endif()

option(BUILD_DEBUG "option for debug / release build." TRUE)

if(BUILD_DEBUG)
  set(CMAKE_BUILD_TYPE "Debug")
else()
  set(CMAKE_BUILD_TYPE "Release")
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} lower_build_type)
option(FEMTOLOG_BUILD_SHARED "build libraries as shared" FALSE)
option(FEMTOLOG_BUILD_BENCHMARK "build benchmark module" FALSE)
option(FEMTOLOG_BUILD_TESTING "build testing module" FALSE)
option(FEMTOLOG_INSTALL_HEADERS "install headers" FALSE)
option(FEMTOLOG_INSTALL_LIBS "install libraries" FALSE)
option(FEMTOLOG_INSTALL_BENCHMARK "install benchmark module" FALSE)
option(FEMTOLOG_INSTALL_TESTING "install testing module" FALSE)
option(FEMTOLOG_ENABLE_RUN_BENCHMARK_POST_BUILD "enable running benchmarks post build" FALSE)
option(FEMTOLOG_ENABLE_RUN_TESTING_POST_BUILD "enable running tests post build" TRUE)

option(FEMTOLOG_ENABLE_BENCH_QUILL "Enable spdlog for benchmark" TRUE)
option(FEMTOLOG_ENABLE_BENCH_SPDLOG "Enable spdlog for benchmark" TRUE)

string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} lower_arch)
string(TOLOWER ${CMAKE_SYSTEM_NAME} lower_os_name)

set(TARGET_ARCH_NAME ${lower_arch} CACHE STRING "target architecture name in lower case")
set(TARGET_OS_NAME ${lower_os_name} CACHE STRING "target os name in lower case")

string(TOLOWER ${CMAKE_HOST_SYSTEM_PROCESSOR} HOST_ARCH_NAME)
string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} HOST_OS_NAME)

option(FEMTOLOG_ENABLE_VERBOSE "enable detailed build information" TRUE)
option(FEMTOLOG_ENABLE_LTO "enable link time optimization" TRUE)
option(FEMTOLOG_ENABLE_NATIVE_ARCH "enable optimization for native architecture" TRUE)

option(FEMTOLOG_ENABLE_BUILD_REPORT "generate the report for optimization, build time, etc." TRUE)
option(FEMTOLOG_ENABLE_COVERAGE "enable coverage using -fprofile-instr-generate and llvm-cov. runs main app and test on post build if available." TRUE)
option(FEMTOLOG_ENABLE_OPTIMIZATION_REPORT "generate optimization report" TRUE)
option(FEMTOLOG_ENABLE_XRAY "enable llvm-xray. not compatible with windows build, debug build, address sanitizers. runs main app and test on post build if available." TRUE)
option(FEMTOLOG_ENABLE_SANITIZERS "enable address and undefined sanitizers" TRUE)
option(FEMTOLOG_ENABLE_AVX2 "enable avx2 if available" TRUE)

option(FEMTOLOG_ENABLE_WARNINGS_AS_ERRORS "treat warnings as errors" TRUE)

if(FEMTOLOG_ENABLE_OPTIMIZATION_REPORT AND HOST_OS_NAME MATCHES "windows")
  message(WARNING "FEMTOLOG_ENABLE_OPTIMIZATION_REPORT on Windows build is not supported.\n"
    "set FEMTOLOG_ENABLE_OPTIMIZATION_REPORT to false forcely.")
  set(FEMTOLOG_ENABLE_OPTIMIZATION_REPORT FALSE)
endif()

if(FEMTOLOG_ENABLE_COVERAGE AND MINGW_BUILD)
  message(WARNING "FEMTOLOG_ENABLE_COVERAGE for MinGW build is not supported.\n"
    "set FEMTOLOG_ENABLE_COVERAGE to false forcely.")
  set(FEMTOLOG_ENABLE_COVERAGE FALSE)
endif()

if(FEMTOLOG_ENABLE_XRAY AND MINGW_BUILD)
  message(WARNING "FEMTOLOG_ENABLE_XRAY for MinGW build is not supported.\n"
    "set FEMTOLOG_ENABLE_XRAY to false forcely.")
  set(FEMTOLOG_ENABLE_XRAY FALSE)
endif()

if(FEMTOLOG_ENABLE_XRAY AND TARGET_OS_NAME MATCHES "windows")
  message(WARNING "FEMTOLOG_ENABLE_XRAY for Windows build is not supported.\n"
    "set FEMTOLOG_ENABLE_XRAY to false forcely.")
  set(FEMTOLOG_ENABLE_XRAY FALSE)
endif()

if(FEMTOLOG_ENABLE_AVX2 AND TARGET_OS_NAME MATCHES "darwin")
  message(WARNING "FEMTOLOG_ENABLE_AVX2 for Darwin build is not supported.\n"
    "set FEMTOLOG_ENABLE_AVX2 to false forcely.")
  set(FEMTOLOG_ENABLE_AVX2 FALSE)
endif()

if(FEMTOLOG_ENABLE_XRAY AND FEMTOLOG_ENABLE_SANITIZERS)
  message(WARNING "FEMTOLOG_ENABLE_XRAY and FEMTOLOG_ENABLE_SANITIZERS cannot be enabled at the same time.\n"
    "set FEMTOLOG_ENABLE_SANITIZERS to false forcely.")
  set(FEMTOLOG_ENABLE_SANITIZERS FALSE)
endif()

if(FEMTOLOG_ENABLE_LTO AND BUILD_DEBUG)
  message(WARNING "FEMTOLOG_ENABLE_LTO in Debug build is not recommended.\n"
    "set FEMTOLOG_ENABLE_LTO to false forcely.")
  set(FEMTOLOG_ENABLE_LTO FALSE)
endif()

if(FEMTOLOG_ENABLE_XRAY AND NOT BUILD_DEBUG)
  message(WARNING "FEMTOLOG_ENABLE_XRAY in Release build is not recommended.\n"
    "set FEMTOLOG_ENABLE_XRAY to false forcely.")
  set(FEMTOLOG_ENABLE_XRAY FALSE)
endif()

if(FEMTOLOG_ENABLE_COVERAGE AND NOT BUILD_DEBUG)
  message(WARNING "FEMTOLOG_ENABLE_COVERAGE in Release build is not recommended.\n"
    "set FEMTOLOG_ENABLE_COVERAGE to false forcely.")
  set(FEMTOLOG_ENABLE_COVERAGE FALSE)
endif()

set(ADDITIONAL_INCLUDE_DIRECTORIES "" CACHE STRING "additional include directories to apply whole the project")
set(ADDITIONAL_LINK_DIRECTORIES "" CACHE STRING "additional link directories to apply whole the project")
set(ADDITIONAL_LINK_LIBRARIES "" CACHE STRING "additional link libraries to apply whole the project")

include(utils)

include(flags)
setup_flags()

include(libraries)
setup_llvm()
setup_zlib()
setup_fmtlib()

include(package)
setup_package()

include(setup_module)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(REMOVE_RECURSE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
file(REMOVE_RECURSE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
file(REMOVE_RECURSE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(REMOVE_RECURSE ${CMAKE_PDB_OUTPUT_DIRECTORY})

list(APPEND FEMTOLOG_INCLUDE_DIRECTORIES
  ${PROJECT_SOURCE_DIR}
  ${LLVM_INCLUDE_DIRS}
  ${TOML11_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIR}
  ${FMTLIB_INCLUDE_DIR}
  ${ADDITIONAL_INCLUDE_DIRECTORIES}
  ${PROJECT_SOURCE_DIR}/include
)

list(APPEND FEMTOLOG_LINK_DIRECTORIES
  ${LLVM_LIBRARY_DIRS}
  ${ADDITIONAL_LINK_DIRECTORIES}
)

list(APPEND FEMTOLOG_LINK_LIBRARIES
  ${MINGW_LINK_LIBRARIES}
  ${WINDOWS_LINK_LIBRARIES}
  ${ADDITIONAL_LINK_LIBRARIES}
)

if(FEMTOLOG_ENABLE_VERBOSE)
  message("${Green}
-=-=-=-=-=-= CMake build status -=-=-=-=-=-=-
cmake version : ${CMAKE_VERSION}
project name: ${CMAKE_PROJECT_NAME}, project version: ${CMAKE_PROJECT_VERSION}
main library name: ${MAIN_LIB_NAME}
c language version: ${CMAKE_C_STANDARD}, c++ language version: ${CMAKE_CXX_STANDARD}
c compiler: ${CMAKE_C_COMPILER} (ver: ${CMAKE_C_COMPILER_VERSION})
c++ compiler: ${CMAKE_CXX_COMPILER} (ver: ${CMAKE_CXX_COMPILER_VERSION})
build type: ${CMAKE_BUILD_TYPE}
host os name: ${HOST_OS_NAME}
host arch name: ${HOST_ARCH_NAME}
target os name: ${TARGET_OS_NAME}
target arch name: ${TARGET_ARCH_NAME}
include directories: ${FEMTOLOG_INCLUDE_DIRECTORIES}
link directories: ${FEMTOLOG_LINK_DIRECTORIES}
link libraries: ${FEMTOLOG_LINK_LIBRARIES}
project compile options: ${PROJECT_COMPILE_OPTIONS}
project link options: ${FEMTOLOG_LINK_OPTIONS}
install prefix: ${CMAKE_INSTALL_PREFIX}
build resources dir: ${BUILD_RESOURCES_DIR}
is mingw build: ${MINGW_BUILD}
toolchain file: ${CMAKE_TOOLCHAIN_FILE}
build debug: ${BUILD_DEBUG}
build shared: ${FEMTOLOG_BUILD_SHARED}
build benchmark: ${FEMTOLOG_BUILD_BENCHMARK}
build testing: ${FEMTOLOG_BUILD_TESTING}
install headers: ${FEMTOLOG_INSTALL_HEADERS}
install libs: ${FEMTOLOG_INSTALL_LIBS}
install benchmark: ${FEMTOLOG_INSTALL_BENCHMARK}
install testing: ${FEMTOLOG_INSTALL_TESTING}
enable run benchmark post build: ${FEMTOLOG_ENABLE_RUN_BENCHMARK_POST_BUILD}
enable run testing post build: ${FEMTOLOG_ENABLE_RUN_TESTING_POST_BUILD}

enable bench quill: ${FEMTOLOG_ENABLE_BENCH_QUILL}
enable bench spdlog: ${FEMTOLOG_ENABLE_BENCH_SPDLOG}

enable verbose: ${FEMTOLOG_ENABLE_VERBOSE}
enable lto: ${FEMTOLOG_ENABLE_LTO}
enable native arch: ${FEMTOLOG_ENABLE_NATIVE_ARCH}
generate build report: ${FEMTOLOG_ENABLE_BUILD_REPORT}
enable profile: ${FEMTOLOG_ENABLE_COVERAGE}
enable optimization report: ${FEMTOLOG_ENABLE_OPTIMIZATION_REPORT}
enable xray: ${FEMTOLOG_ENABLE_XRAY}
enable sanitizers: ${FEMTOLOG_ENABLE_SANITIZERS}
enable avx2: ${FEMTOLOG_ENABLE_AVX2}
warnings as errors: ${FEMTOLOG_ENABLE_WARNINGS_AS_ERRORS}
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-${ColourReset}")
  print_all_build_flags()
endif()

add_subdirectory(build)

add_subdirectory(core)
add_subdirectory(logging)

add_subdirectory(femtolog)
add_subdirectory(include)

if(FEMTOLOG_BUILD_TESTING)
  setup_gtest()
  enable_testing()
  add_subdirectory(testing)
endif()

if(FEMTOLOG_BUILD_BENCHMARK)
  setup_google_benchmark()
  add_subdirectory(bench)
endif()
