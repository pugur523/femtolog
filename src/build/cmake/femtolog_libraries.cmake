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
    set(INSTALL_GTEST FALSE CACHE BOOL "" FORCE)
    set(BUILD_GMOCK TRUE CACHE BOOL "" FORCE)

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt TRUE CACHE BOOL "" FORCE)

    add_subdirectory(${GTEST_ROOT_DIR})

    # Do not link `gtest_main` and `gmock_main` otherwise the testing will contain duplicate entrypoints.
    set(GTEST_LIBRARIES gtest)
    target_compile_options(${GTEST_LIBRARIES} PRIVATE ${FEMTOLOG_COMPILE_OPTIONS})
    target_link_options(${GTEST_LIBRARIES} PRIVATE ${FEMTOLOG_LINK_OPTIONS})
    target_link_libraries(${GTEST_LIBRARIES} PRIVATE ${FEMTOLOG_LINK_LIBRARIES})

    if(BUILD_GMOCK)
      set(GMOCK_LIBRARIES gmock)
      target_compile_options(${GMOCK_LIBRARIES} PRIVATE ${FEMTOLOG_COMPILE_OPTIONS})
      target_link_options(${GMOCK_LIBRARIES} PRIVATE ${FEMTOLOG_LINK_OPTIONS})
      target_link_libraries(${GMOCK_LIBRARIES} PRIVATE ${FEMTOLOG_LINK_LIBRARIES})
    endif()
  endif()
endmacro()

macro(femtolog_setup_google_benchmark)
  if(NOT FEMTOLOG_USE_EXTERNAL_GOOGLE_BENCHMARK)
    set(GOOGLE_BENCHMARK_DIR ${THIRD_PARTY_DIR}/google_benchmark)
    set(GOOGLE_BENCHMARK_INCLUDE_DIR ${GOOGLE_BENCHMARK_DIR}/include)

    set(BENCHMARK_ENABLE_TESTING FALSE)
    set(BENCHMARK_ENABLE_EXCEPTIONS FALSE)
    set(BENCHMARK_ENABLE_LTO ${FEMTOLOG_ENABLE_LTO})
    set(BENCHMARK_USE_LIBCXX TRUE)
    set(BENCHMARK_ENABLE_WERROR ${FEMTOLOG_ENABLE_WARNINGS_AS_ERRORS})
    set(BENCHMARK_FORCE_WERROR ${FEMTOLOG_ENABLE_WARNINGS_AS_ERRORS})

    set(BENCHMARK_ENABLE_INSTALL FALSE)
    set(BENCHMARK_INSTALL_DOCS FALSE)

    set(BENCHMARK_ENABLE_GTEST_TESTS FALSE)

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

    set(GOOGLE_BENCHMARK_LIBRARIES benchmark)

    target_compile_options(${GOOGLE_BENCHMARK_LIBRARIES} PRIVATE ${FEMTOLOG_COMPILE_OPTIONS})
    target_link_options(${GOOGLE_BENCHMARK_LIBRARIES} PRIVATE ${FEMTOLOG_LINK_OPTIONS})
    target_link_libraries(${GOOGLE_BENCHMARK_LIBRARIES} PRIVATE ${FEMTOLOG_LINK_LIBRARIES})
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

  if(FEMTOLOG_ENABLE_LLVM_UNWIND)
    list(APPEND FEMTOLOG_COMPILE_DEFINITIONS FEMTOLOG_ENABLE_LLVM_UNWIND=1)
  else()
    list(APPEND FEMTOLOG_COMPILE_DEFINITIONS FEMTOLOG_ENABLE_LLVM_UNWIND=0)
  endif()

  if(NOT APPLE)
    if(FEMTOLOG_ENABLE_LLVM_UNWIND)
      list(APPEND FEMTOLOG_LINK_OPTIONS -unwindlib=libunwind -rtlib=compiler-rt -Wl,-lunwind)
      list(APPEND FEMTOLOG_LINK_LIBRARIES c++abi unwind unwind-ptrace unwind-x86_64)
    endif()
  endif()

  if(MINGW_BUILD)
    list(APPEND FEMTOLOG_LINK_OPTIONS -Wl,-Bstatic -lc++ -lc++abi -lunwind -Wl,-Bdynamic)
  endif()
endmacro()

macro(femtolog_setup_zlib)
  if(NOT FEMTOLOG_USE_EXTERNAL_ZLIB)
    set(ZLIB_DIR ${THIRD_PARTY_DIR}/zlib)
    set(ZLIB_INCLUDE_DIR ${ZLIB_DIR})

    set(ZLIB_BUILD_TESTING FALSE CACHE BOOL "" FORCE)
    set(ZLIB_BUILD_STATIC TRUE CACHE BOOL "" FORCE)
    set(ZLIB_BUILD_SHARED FALSE CACHE BOOL "" FORCE)
    set(ZLIB_BUILD_MINIZIP FALSE CACHE BOOL "" FORCE)
    set(ZLIB_BUILD_TESTING FALSE CACHE BOOL "" FORCE)
    set(ZLIB_INSTALL FALSE CACHE BOOL "" FORCE)

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

    if(${FEMTOLOG_INSTALL_LIBS})
      set(FMT_INSTALL TRUE CACHE BOOL "" FORCE)
    else()
      set(FMT_INSTALL FALSE CACHE BOOL "" FORCE)
    endif()

    add_subdirectory(${FMTLIB_DIR})

    set(FMTLIB_LIBRARIES fmt)

    target_compile_options(${FMTLIB_LIBRARIES} PRIVATE ${FEMTOLOG_COMPILE_OPTIONS})
    target_link_options(${FMTLIB_LIBRARIES} PRIVATE ${FEMTOLOG_LINK_OPTIONS})
    target_link_libraries(${FMTLIB_LIBRARIES} PRIVATE ${FEMTOLOG_LINK_LIBRARIES})
  endif()
endmacro()
