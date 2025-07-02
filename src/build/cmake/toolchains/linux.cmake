# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Prefer versioned LLVM tools if available
find_program(CLANG_PATH NAMES clang-21 clang REQUIRED)
find_program(CLANGXX_PATH NAMES clang++-21 clang++ REQUIRED)
find_program(LLD_PATH NAMES ld.lld-21 ld.lld lld REQUIRED)
find_program(LLVM_AR_PATH NAMES llvm-ar-21 llvm-ar REQUIRED)
find_program(LLVM_RANLIB_PATH NAMES llvm-ranlib-21 llvm-ranlib REQUIRED)
find_program(LLVM_NM_PATH NAMES llvm-nm-21 llvm-nm REQUIRED)
find_program(LLVM_OBJCOPY_PATH NAMES llvm-objcopy-21 llvm-objcopy REQUIRED)
find_program(LLVM_OBJDUMP_PATH NAMES llvm-objdump-21 llvm-objdump REQUIRED)
find_program(LLVM_STRIP_PATH NAMES llvm-strip-21 llvm-strip REQUIRED)

# Set compiler and tools
set(CMAKE_C_COMPILER "${CLANG_PATH}")
set(CMAKE_CXX_COMPILER "${CLANGXX_PATH}")
set(CMAKE_ASM_COMPILER "${CLANG_PATH}")
set(CMAKE_LINKER "${LLD_PATH}")
set(CMAKE_AR "${LLVM_AR_PATH}")
set(CMAKE_RANLIB "${LLVM_RANLIB_PATH}")
set(CMAKE_NM "${LLVM_NM_PATH}")
set(CMAKE_OBJCOPY "${LLVM_OBJCOPY_PATH}")
set(CMAKE_OBJDUMP "${LLVM_OBJDUMP_PATH}")
set(CMAKE_STRIP "${LLVM_STRIP_PATH}")

# Linker flags - use lld
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")

# Search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
