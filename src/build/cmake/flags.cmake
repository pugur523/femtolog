# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

macro(setup_windows_flags)
  # Enable color and use libc++
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics -fdiagnostics-color=always" PARENT_SCOPE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics -fdiagnostics-color=always" PARENT_SCOPE)

  # Base flags - enable warnings
  list(APPEND PROJECT_COMPILE_OPTIONS /W4 /clang:-Wall /clang:-Wextra /clang:-Wpedantic)

  if(FEMTOLOG_ENABLE_WARNINGS_AS_ERRORS)
    list(APPEND PROJECT_COMPILE_OPTIONS /WX /clang:-Werror)
  endif()

  # Debug configuration
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi /DDEBUG /ZH:SHA_256")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /DDEBUG /ZH:SHA_256")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG")

  # Release configuration
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG /Gw /Gy /Zc:dllexportInlines- /clang:-fno-exceptions")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG /Gw /Gy /Zc:dllexportInlines- /clang:-fno-exceptions")

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /permissive- /Zc:__cplusplus /Zc:inline /Zc:strictStrings /Zc:alignedNew /Zc:sizedDealloc /Zc:threadSafeInit")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /Zc:__cplusplus /Zc:inline /Zc:strictStrings /Zc:alignedNew /Zc:sizedDealloc /Zc:threadSafeInit")

  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

  # Link time optimization
  if(FEMTOLOG_ENABLE_LTO)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Gw /Gy /clang:-flto=thin")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Gw /Gy /clang:-flto=thin")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF /MAP:${CMAKE_BINARY_DIR}/link.map")
  endif()

  # Native architecture optimization
  if(FEMTOLOG_ENABLE_NATIVE_ARCH)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /arch:AVX2")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:AVX2")
  endif()

  if(FEMTOLOG_ENABLE_BUILD_REPORT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /clang:-ftime-trace")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clang:-ftime-trace")
  endif()

  if(FEMTOLOG_ENABLE_OPTIMIZATION_REPORT)
    list(APPEND PROJECT_COMPILE_OPTIONS "/clang:-fsave-optimization-record;/clang:-fdebug-compilation-dir=.;/clang:-Rpass='.*';/clang:-Rpass-missed='.*';/clang:-Rpass-analysis='.*'")
  endif()

  list(APPEND WINDOWS_LINK_LIBRARIES "dbghelp")
endmacro()

macro(setup_unix_flags)
  # Enable color and use libc++
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always -stdlib=libc++")

  # Base flags - enable warnings
  list(APPEND PROJECT_COMPILE_OPTIONS -Wall -Wextra -Wpedantic -fno-common)

  if(FEMTOLOG_ENABLE_WARNINGS_AS_ERRORS)
    list(APPEND PROJECT_COMPILE_OPTIONS -Werror)
  endif()

  # Debug configuration
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -fno-inline -fmacro-backtrace-limit=0 -frtti -fno-omit-frame-pointer")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fno-inline -fmacro-backtrace-limit=0 -frtti -fno-omit-frame-pointer")

  # Release configuration
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -ffunction-sections -fdata-sections -fstack-protector-strong -D_FORTIFY_SOURCE=2 -ftrivial-auto-var-init=zero -fno-rtti -fomit-frame-pointer -fno-exceptions")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffunction-sections -fdata-sections -fstack-protector-strong -D_FORTIFY_SOURCE=2 -ftrivial-auto-var-init=zero -fno-rtti -fomit-frame-pointer -fno-exceptions")

  # Avoid mingw
  if(NOT MINGW_BUILD)
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -rdynamic")

    if(NOT TARGET_OS_NAME MATCHES "darwin")
      set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fstack-clash-protection")
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-clash-protection")
    endif()
  endif()

  # Hide symbols by default, this will break stack trace.
  # set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  # set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

  # Link time optimization
  if(FEMTOLOG_ENABLE_LTO)
    if(NOT MINGW_BUILD)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto=thin -funified-lto")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=thin -funified-lto")

      if(NOT TARGET_OS_NAME MATCHES "darwin")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--lto=thin -funified-lto")
      endif()
    endif()

    if(TARGET_OS_NAME MATCHES "darwin")
      # for macos (ld64.lld)
      set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,-dead_strip")
    else()
      set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")
    endif()
  endif()

  # Native architecture optimization
  if(FEMTOLOG_ENABLE_NATIVE_ARCH)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native")
  endif()

  if(FEMTOLOG_ENABLE_BUILD_REPORT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftime-trace")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Map=${CMAKE_BINARY_DIR}/link.map")
  endif()

  if(FEMTOLOG_ENABLE_OPTIMIZATION_REPORT)
    list(APPEND PROJECT_COMPILE_OPTIONS "-fsave-optimization-record;-fdebug-compilation-dir=.;-Rpass='.*';-Rpass-missed='.*';-Rpass-analysis='.*'")
  endif()

  # Sanitize configuration
  if(FEMTOLOG_ENABLE_SANITIZERS AND BUILD_DEBUG AND NOT MINGW_BUILD)
    set(SAN_FLAGS "-fsanitize=address,undefined")

    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${SAN_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SAN_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${SAN_FLAGS}")
  endif()

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmacro-prefix-map=${PROJECT_ROOT_DIR}/=")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmacro-prefix-map=${PROJECT_ROOT_DIR}/=")
endmacro()

macro(setup_mingw_flags)
  set(MINGW_LINK_LIBRARIES "dbghelp")
endmacro()

macro(setup_apple_flags)
  execute_process(
    COMMAND xcrun --sdk macosx --show-sdk-path
    OUTPUT_VARIABLE MACOS_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  add_compile_options(-isysroot ${MACOS_SDK_PATH} -I${MACOS_SDK_PATH}/usr/include)
endmacro()

macro(setup_common_flags)
  # Profiling with llvm-coverage
  if(FEMTOLOG_ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
  endif()

  # Profiling with llvm-xray
  if(FEMTOLOG_ENABLE_XRAY)
    list(APPEND PROJECT_COMPILE_OPTIONS -fxray-instrument -fxray-instrumentation-bundle=function -fxray-instruction-threshold=1)
    list(APPEND FEMTOLOG_LINK_OPTIONS -fxray-instrument -fxray-instrumentation-bundle=function -fxray-instruction-threshold=1)

    if(NOT TARGET_OS_NAME MATCHES "darwin")
      list(APPEND PROJECT_COMPILE_OPTIONS -fxray-shared)
      list(APPEND FEMTOLOG_LINK_OPTIONS -fxray-shared)
    endif()

    add_compile_definitions(FEMTOLOG_ENABLE_XRAY=1)
  else()
    add_compile_definitions(FEMTOLOG_ENABLE_XRAY=0)
  endif()

  if(FEMTOLOG_ENABLE_AVX2)
    include(CheckCXXSourceRuns)

    if(CMAKE_CROSSCOMPILING)
      message(STATUS "Cross-compiling detected. Skipping runtime AVX2 check.")

      set(HAS_AVX2 TRUE)
    else()
      set(AVX2_TEST_CODE "
      #include <immintrin.h>
      int main() {
          __m256i a = _mm256_set1_epi32(1);
          __m256i b = _mm256_add_epi32(a, a);
          return 0;
      }
    ")

      set(CMAKE_REQUIRED_FLAGS "-mavx2")
      check_cxx_source_runs("${AVX2_TEST_CODE}" HAS_AVX2)
    endif()

    if(HAS_AVX2)
      message(STATUS "AVX2 support detected")
      add_compile_definitions(FEMTOLOG_ENABLE_AVX2=1)
      list(APPEND PROJECT_COMPILE_OPTIONS -mavx2)
    else()
      add_compile_definitions(FEMTOLOG_ENABLE_AVX2=0)
      message(STATUS "AVX2 support not available")
    endif()
  else()
    add_compile_definitions(FEMTOLOG_ENABLE_AVX2=0)
  endif()
endmacro()

macro(setup_flags)
  include(CheckCXXCompilerFlag)
  string(TOUPPER ${MAIN_LIB_NAME} UPPER_PROJECT_NAME)

  if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang" OR NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "Unknown compiler: C=${CMAKE_C_COMPILER_ID}, CXX=${CMAKE_CXX_COMPILER_ID}")
  endif()

  if(MSVC AND TARGET_OS_NAME MATCHES "windows")
    setup_windows_flags()
  elseif(NOT MSVC)
    setup_unix_flags()
  endif()

  if(MINGW_BUILD)
    setup_mingw_flags()
  endif()

  if(APPLE)
    setup_apple_flags()
  endif()

  setup_common_flags()
endmacro()

function(print_all_build_flags)
  message("${Cyan}
-=-=-=-= Complete Flag Configuration -=-=-=-=-
CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}
CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}
CMAKE_C_FLAGS_DEBUG: ${CMAKE_C_FLAGS_DEBUG}
CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}
CMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}
CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}
CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}
CMAKE_SHARED_LINKER_FLAGS_DEBUG: ${CMAKE_SHARED_LINKER_FLAGS_DEBUG}
CMAKE_SHARED_LINKER_FLAGS_RELEASE: ${CMAKE_SHARED_LINKER_FLAGS_RELEASE}
PROJECT_COMPILE_OPTIONS: ${PROJECT_COMPILE_OPTIONS}
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
${ColourReset}")
endfunction()

# Helper function to reset all flags (for testing)
function(reset_all_flags)
  set(CMAKE_C_FLAGS "" PARENT_SCOPE)
  set(CMAKE_CXX_FLAGS "" PARENT_SCOPE)
  set(CMAKE_C_FLAGS_DEBUG "" PARENT_SCOPE)
  set(CMAKE_CXX_FLAGS_DEBUG "" PARENT_SCOPE)
  set(CMAKE_C_FLAGS_RELEASE "" PARENT_SCOPE)
  set(CMAKE_CXX_FLAGS_RELEASE "" PARENT_SCOPE)
  set(CMAKE_SHARED_LINKER_FLAGS "" PARENT_SCOPE)
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "" PARENT_SCOPE)
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "" PARENT_SCOPE)
  message(STATUS "All flags have been reset")
endfunction()
