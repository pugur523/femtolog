# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# MinGW toolchain prefix
set(MINGW_PREFIX "x86_64-w64-mingw32")

# Prefer versioned LLVM tools if available
find_program(CLANG_PATH NAMES ${MINGW_PREFIX}-clang REQUIRED)
find_program(CLANGXX_PATH NAMES ${MINGW_PREFIX}-clang++ REQUIRED)
find_program(LD_PATH NAMES ${MINGW_PREFIX}-ld REQUIRED)
find_program(LLVM_AR_PATH NAMES ${MINGW_PREFIX}-llvm-ar REQUIRED)
find_program(LLVM_RANLIB_PATH NAMES ${MINGW_PREFIX}-llvm-ranlib REQUIRED)
find_program(NM_PATH NAMES ${MINGW_PREFIX}-nm REQUIRED)
find_program(OBJCOPY_PATH NAMES ${MINGW_PREFIX}-objcopy REQUIRED)
find_program(OBJDUMP_PATH NAMES ${MINGW_PREFIX}-objdump REQUIRED)
find_program(RC_PATH NAMES ${MINGW_PREFIX}-windres REQUIRED)
find_program(STRIP_PATH NAMES ${MINGW_PREFIX}-strip REQUIRED)

# Set compiler and tools
set(CMAKE_C_COMPILER "${CLANG_PATH}")
set(CMAKE_CXX_COMPILER "${CLANGXX_PATH}")
set(CMAKE_ASM_COMPILER "${CLANG_PATH}")
set(CMAKE_LINKER "${LD_PATH}")
set(CMAKE_AR "${LLVM_AR_PATH}")
set(CMAKE_RANLIB "${LLVM_RANLIB_PATH}")
set(CMAKE_NM "${NM_PATH}")
set(CMAKE_OBJCOPY "${OBJCOPY_PATH}")
set(CMAKE_OBJDUMP "${OBJDUMP_PATH}")
set(CMAKE_RC_COMPILER "${RC_PATH}")
set(CMAKE_STRIP "${STRIP_PATH}")

set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")

# Search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
