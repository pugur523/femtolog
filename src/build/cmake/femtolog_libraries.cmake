# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

macro(femtolog_setup_gtest)
  if(NOT FEMTOLOG_USE_EXTERNAL_GTEST)
    set(GTEST_ROOT_DIR ${THIRD_PARTY_DIR}/gtest)
    set(GTEST_DIR ${GTEST_ROOT_DIR}/googletest)
    set(GTEST_INCLUDE_DIR ${GTEST_DIR}/include)

    set(GMOCK_DIR ${GTEST_ROOT_DIR}/googlemock)
    set(GMOCK_INCLUDE_DIR ${GMOCK_DIR}/include)

    # https://google.github.io/googletest/quickstart-cmake.html
    set(INSTALL_GTEST FALSE)
    set(BUILD_GMOCK TRUE)

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt TRUE CACHE BOOL "" FORCE)

    add_subdirectory(${GTEST_ROOT_DIR})

    # Do not link `gtest_main` and `gmock_main` otherwise the testing will contain duplicate entrypoints.
    set(GTEST_LIBRARIES gtest)

    if(BUILD_GMOCK)
      set(GMOCK_LIBRARIES gmock)
    endif()
  endif()
endmacro()

macro(femtolog_setup_google_benchmark)
  if(NOT FEMTOLOG_USE_EXTERNAL_GOOGLE_BENCHMARK)
    set(GOOGLE_BENCHMARK_DIR ${THIRD_PARTY_DIR}/google_benchmark)
    set(GOOGLE_BENCHMARK_INCLUDE_DIR ${GOOGLE_BENCHMARK_DIR}/include)

    set(BENCHMARK_ENABLE_TESTING FALSE)
    set(BENCHMARK_ENABLE_EXCEPTIONS TRUE)
    set(BENCHMARK_FEMTOLOG_ENABLE_LTO ${FEMTOLOG_ENABLE_LTO})
    set(BENCHMARK_USE_LIBCXX TRUE)
    set(BENCHMARK_ENABLE_WERROR ${FEMTOLOG_ENABLE_WARNINGS_AS_ERRORS})
    set(BENCHMARK_FORCE_WERROR ${FEMTOLOG_ENABLE_WARNINGS_AS_ERRORS})

    set(BENCHMARK_ENABLE_INSTALL FALSE)
    set(BENCHMARK_INSTALL_DOCS FALSE)

    if(MINGW_BUILD)
      set(HAVE_STD_REGEX FALSE)
      set(HAVE_STEADY_CLOCK FALSE)
      set(HAVE_THREAD_SAFETY_ATTRIBUTES FALSE)
    endif()

    # clang-cl's `/GL` option will compete with `/clang:-flto=thin`
    if(HOST_OS_NAME MATCHES "windows" AND FEMTOLOG_ENABLE_LTO)
      set(BENCHMARK_FEMTOLOG_ENABLE_LTO FALSE)
    endif()

    add_subdirectory(${GOOGLE_BENCHMARK_DIR})

    set(GOOGLE_BENCHMARK_LIBRARIES benchmark::benchmark)
  endif()
endmacro()

macro(femtolog_setup_llvm)
  # Windows: manually specify llvm paths
  if(NOT TARGET_OS_NAME MATCHES "windows")
    find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config-21 llvm-config)

    if(NOT LLVM_CONFIG_EXECUTABLE)
      message(FATAL_ERROR "llvm-config not found. Please install LLVM development tools.")
    endif()

    execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
      OUTPUT_VARIABLE LLVM_INCLUDE_DIRS
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
      OUTPUT_VARIABLE LLVM_LIBRARY_DIRS
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --version
      OUTPUT_VARIABLE LLVM_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()

  if(MINGW_BUILD)
    list(APPEND FEMTOLOG_LINK_OPTIONS -Wl,-Bstatic -lc++ -lc++abi -lunwind -Wl,-Bdynamic)
  endif()
endmacro()

macro(femtolog_setup_zlib)
  if(NOT FEMTOLOG_USE_EXTERNAL_ZLIB)
    set(ZLIB_DIR ${THIRD_PARTY_DIR}/zlib)
    set(ZLIB_INCLUDE_DIR ${ZLIB_DIR})

    set(ZLIB_FEMTOLOG_BUILD_TESTING FALSE)
    set(ZLIB_BUILD_STATIC TRUE)
    set(ZLIB_FEMTOLOG_BUILD_SHARED FALSE)
    set(ZLIB_BUILD_MINIZIP FALSE)
    set(ZLIB_BUILD_TESTING FALSE)
    set(ZLIB_INSTALL FALSE)

    set(ZLIB_LIBRARIES zlibstatic)

    add_subdirectory(${ZLIB_DIR})

    target_compile_options(${ZLIB_LIBRARIES} PRIVATE "-Wno-implicit-function-declaration")

    set_target_properties(${ZLIB_LIBRARIES}
      PROPERTIES
      POSITION_INDEPENDENT_CODE TRUE
    )
  endif()
endmacro()

macro(femtolog_setup_fmtlib)
  if(NOT FEMTOLOG_USE_EXTERNAL_FMTLIB)
    set(FMTLIB_DIR ${THIRD_PARTY_DIR}/fmtlib)
    set(FMTLIB_INCLUDE_DIR ${FMTLIB_DIR}/include)

    set(FMT_INSTALL FALSE)

    set(FMTLIB_LIBRARIES fmt::fmt)

    add_subdirectory(${FMTLIB_DIR})
  endif()
endmacro()