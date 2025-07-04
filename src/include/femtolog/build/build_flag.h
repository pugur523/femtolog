// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BUILD_BUILD_FLAG_H_
#define INCLUDE_FEMTOLOG_BUILD_BUILD_FLAG_H_

// ==================
// Platform Detection
// ==================

#define FEMTOLOG_IS_WINDOWS 0
#define FEMTOLOG_IS_UNIX 0
#define FEMTOLOG_IS_MAC 0
#define FEMTOLOG_IS_IOS 0
#define FEMTOLOG_IS_ANDROID 0
#define FEMTOLOG_IS_LINUX 0
#define FEMTOLOG_IS_BSD 0
#define FEMTOLOG_IS_CYGWIN 0
#define FEMTOLOG_IS_SOLARIS 0

// Override based on actual platform
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
#undef FEMTOLOG_IS_WINDOWS
#define FEMTOLOG_IS_WINDOWS 1

#elif defined(__APPLE__)
#include <TargetConditionals.h>
#undef FEMTOLOG_IS_UNIX
#define FEMTOLOG_IS_UNIX 1

#if TARGET_OS_MAC
#undef FEMTOLOG_IS_MAC
#define FEMTOLOG_IS_MAC 1

#elif TARGET_OS_IPHONE
#undef FEMTOLOG_IS_IOS
#define FEMTOLOG_IS_IOS 1

#else
#undef FEMTOLOG_IS_MAC
#define FEMTOLOG_IS_MAC 1

#endif

#elif defined(__linux__) || defined(__linux) || defined(linux)
#undef FEMTOLOG_IS_UNIX
#define FEMTOLOG_IS_UNIX 1

#if defined(__ANDROID__)
#undef FEMTOLOG_IS_ANDROID
#define FEMTOLOG_IS_ANDROID 1

#else
#undef FEMTOLOG_IS_LINUX
#define FEMTOLOG_IS_LINUX 1

#endif

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__DragonFly__)
#undef FEMTOLOG_IS_UNIX
#undef FEMTOLOG_IS_BSD
#define FEMTOLOG_IS_UNIX 1
#define FEMTOLOG_IS_BSD 1

#elif defined(__CYGWIN__)
#undef FEMTOLOG_IS_UNIX
#undef FEMTOLOG_IS_CYGWIN
#define FEMTOLOG_IS_UNIX 1
#define FEMTOLOG_IS_CYGWIN 1

#elif defined(__sun) && defined(__SVR4)
#undef FEMTOLOG_IS_UNIX
#undef FEMTOLOG_IS_SOLARIS
#define FEMTOLOG_IS_UNIX 1
#define FEMTOLOG_IS_SOLARIS 1

#else
#error "Unsupported platform detected."

#endif

// ==================
// Compiler Detection
// ==================

#if defined(_MSC_VER)
#define FEMTOLOG_COMPILER_MSVC 1
#define FEMTOLOG_COMPILER_GCC 0
#define FEMTOLOG_COMPILER_CLANG 0
#define FEMTOLOG_COMPILER_INTEL 0
#define FEMTOLOG_COMPILER_MINGW 0

#elif defined(__clang__)
#define FEMTOLOG_COMPILER_MSVC 0
#define FEMTOLOG_COMPILER_GCC 0
#define FEMTOLOG_COMPILER_CLANG 1
#define FEMTOLOG_COMPILER_INTEL 0
#define FEMTOLOG_COMPILER_MINGW 0

#elif defined(__INTEL_COMPILER) || defined(__ICC)
#define FEMTOLOG_COMPILER_MSVC 0
#define FEMTOLOG_COMPILER_GCC 0
#define FEMTOLOG_COMPILER_CLANG 0
#define FEMTOLOG_COMPILER_INTEL 1
#define FEMTOLOG_COMPILER_MINGW 0

#elif defined(__MINGW32__) || defined(__MINGW64__)
#define FEMTOLOG_COMPILER_MSVC 0
#define FEMTOLOG_COMPILER_GCC 0
#define FEMTOLOG_COMPILER_CLANG 0
#define FEMTOLOG_COMPILER_INTEL 0
#define FEMTOLOG_COMPILER_MINGW 1

#elif defined(__GNUC__) || defined(__GNUG__)
#define FEMTOLOG_COMPILER_MSVC 0
#define FEMTOLOG_COMPILER_GCC 1
#define FEMTOLOG_COMPILER_CLANG 0
#define FEMTOLOG_COMPILER_INTEL 0
#define FEMTOLOG_COMPILER_MINGW 0

#else
#error "Unsupported compiler detected."

#endif

// ======================
// Architecture Detection
// ======================

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#define FEMTOLOG_ARCH_X64 1
#define FEMTOLOG_ARCH_X86 0
#define FEMTOLOG_ARCH_ARM 0
#define FEMTOLOG_ARCH_ARM64 0
#define FEMTOLOG_ARCH_MIPS 0
#define FEMTOLOG_ARCH_PPC 0
#define FEMTOLOG_ARCH_RISCV 0

#elif defined(__i386) || defined(_M_IX86) || defined(__i386__)
#define FEMTOLOG_ARCH_X64 0
#define FEMTOLOG_ARCH_X86 1
#define FEMTOLOG_ARCH_ARM 0
#define FEMTOLOG_ARCH_ARM64 0
#define FEMTOLOG_ARCH_MIPS 0
#define FEMTOLOG_ARCH_PPC 0
#define FEMTOLOG_ARCH_RISCV 0

#elif defined(__aarch64__) || defined(_M_ARM64)
#define FEMTOLOG_ARCH_X64 0
#define FEMTOLOG_ARCH_X86 0
#define FEMTOLOG_ARCH_ARM 0
#define FEMTOLOG_ARCH_ARM64 1
#define FEMTOLOG_ARCH_MIPS 0
#define FEMTOLOG_ARCH_PPC 0
#define FEMTOLOG_ARCH_RISCV 0

#elif defined(__arm__) || defined(_M_ARM)
#define FEMTOLOG_ARCH_X64 0
#define FEMTOLOG_ARCH_X86 0
#define FEMTOLOG_ARCH_ARM 1
#define FEMTOLOG_ARCH_ARM64 0
#define FEMTOLOG_ARCH_MIPS 0
#define FEMTOLOG_ARCH_PPC 0
#define FEMTOLOG_ARCH_RISCV 0

#elif defined(__mips__) || defined(__mips) || defined(__MIPS__)
#define FEMTOLOG_ARCH_X64 0
#define FEMTOLOG_ARCH_X86 0
#define FEMTOLOG_ARCH_ARM 0
#define FEMTOLOG_ARCH_ARM64 0
#define FEMTOLOG_ARCH_MIPS 1
#define FEMTOLOG_ARCH_PPC 0
#define FEMTOLOG_ARCH_RISCV 0

#elif defined(__powerpc__) || defined(__powerpc) || defined(__ppc__) || \
    defined(__PPC__)
#define FEMTOLOG_ARCH_X64 0
#define FEMTOLOG_ARCH_X86 0
#define FEMTOLOG_ARCH_ARM 0
#define FEMTOLOG_ARCH_ARM64 0
#define FEMTOLOG_ARCH_MIPS 0
#define FEMTOLOG_ARCH_PPC 1
#define FEMTOLOG_ARCH_RISCV 0

#elif defined(__riscv)
#define FEMTOLOG_ARCH_X64 0
#define FEMTOLOG_ARCH_X86 0
#define FEMTOLOG_ARCH_ARM 0
#define FEMTOLOG_ARCH_ARM64 0
#define FEMTOLOG_ARCH_MIPS 0
#define FEMTOLOG_ARCH_PPC 0
#define FEMTOLOG_ARCH_RISCV 1

#else
#define FEMTOLOG_ARCH_X64 0
#define FEMTOLOG_ARCH_X86 0
#define FEMTOLOG_ARCH_ARM 0
#define FEMTOLOG_ARCH_ARM64 0
#define FEMTOLOG_ARCH_MIPS 0
#define FEMTOLOG_ARCH_PPC 0
#define FEMTOLOG_ARCH_RISCV 0

#error "Unknown architecture detected"

#endif

// Pointer size detection
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__) || \
    defined(__powerpc64__) || defined(__mips64) ||                    \
    (defined(__riscv) && __riscv_xlen == 64)
#define FEMTOLOG_ARCH_64BIT 1
#define FEMTOLOG_ARCH_32BIT 0

#else
#define FEMTOLOG_ARCH_64BIT 0
#define FEMTOLOG_ARCH_32BIT 1

#endif

// ====================
// Endianness Detection
// ====================

#if defined(__BYTE_ORDER__)

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define FEMTOLOG_IS_BIG_ENDIAN 1
#define FEMTOLOG_IS_LITTLE_ENDIAN 0

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define FEMTOLOG_IS_BIG_ENDIAN 0
#define FEMTOLOG_IS_LITTLE_ENDIAN 1

#else
#error "Unknown byte order"

#endif

#elif defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || defined(_MIPSEB) || defined(__MIPSEB) ||         \
    defined(__MIPSEB__)
#define FEMTOLOG_IS_BIG_ENDIAN 1
#define FEMTOLOG_IS_LITTLE_ENDIAN 0

#elif defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) ||                 \
    defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL) || \
    defined(__MIPSEL) || defined(__MIPSEL__)
#define FEMTOLOG_IS_BIG_ENDIAN 0
#define FEMTOLOG_IS_LITTLE_ENDIAN 1

#else
// Default to little endian for most modern systems
#define FEMTOLOG_IS_BIG_ENDIAN 0
#define FEMTOLOG_IS_LITTLE_ENDIAN 1
#pragma message("Endianness detection failed, defaulting to little endian")

#endif

// ===================
// Build Configuration
// ===================

#ifdef NDEBUG
#define FEMTOLOG_IS_DEBUG 0
#define FEMTOLOG_IS_RELEASE 1

#else
#define FEMTOLOG_IS_DEBUG 1
#define FEMTOLOG_IS_RELEASE 0

#endif

#if FEMTOLOG_ENABLE_XRAY
#define FEMTOLOG_XRAY_FN [[clang::xray_always_instrument]]
#define FEMTOLOG_XRAY_FN_LOG \
  [[clang::xray_always_instrument, clang::xray_log_args(1)]]

#else
#define FEMTOLOG_XRAY_FN
#define FEMTOLOG_XRAY_FN_LOG

#endif

// ==========================
// Compiler Version Detection
// ==========================

#if FEMTOLOG_COMPILER_MSVC
#define FEMTOLOG_COMPILER_VERSION _MSC_VER

#elif FEMTOLOG_COMPILER_GCC
#define FEMTOLOG_COMPILER_VERSION \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#elif FEMTOLOG_COMPILER_CLANG
#define FEMTOLOG_COMPILER_VERSION \
  (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)

#elif FEMTOLOG_COMPILER_INTEL
#define FEMTOLOG_COMPILER_VERSION __INTEL_COMPILER

#else
#define FEMTOLOG_COMPILER_VERSION 0

#endif

// ===================
// Function Attributes
// ===================

// Inline
#if FEMTOLOG_COMPILER_MSVC
#define FEMTOLOG_FORCE_INLINE __forceinline
#define FEMTOLOG_INLINE __inline

#elif FEMTOLOG_COMPILER_GCC || FEMTOLOG_COMPILER_CLANG
#define FEMTOLOG_FORCE_INLINE inline __attribute__((always_inline))
#define FEMTOLOG_INLINE inline

#else
#define FEMTOLOG_FORCE_INLINE inline
#define FEMTOLOG_INLINE inline

#endif

// No inline
#if FEMTOLOG_COMPILER_MSVC
#define FEMTOLOG_NO_INLINE __declspec(noinline)

#elif FEMTOLOG_COMPILER_GCC || FEMTOLOG_COMPILER_CLANG
#define FEMTOLOG_NO_INLINE __attribute__((noinline))

#else
#define FEMTOLOG_NO_INLINE

#endif

// Memory alignment
#if FEMTOLOG_COMPILER_MSVC
#define FEMTOLOG_ALIGN(x) __declspec(align(x))

#elif FEMTOLOG_COMPILER_GCC || FEMTOLOG_COMPILER_CLANG
#define FEMTOLOG_ALIGN(x) __attribute__((aligned(x)))

#else
#define FEMTOLOG_ALIGN(x)

#endif

// Packed structures
#if FEMTOLOG_COMPILER_MSVC
#define FEMTOLOG_PACKED_STRUCT(name) \
  __pragma(pack(push, 1)) struct name __pragma(pack(pop))

#elif FEMTOLOG_COMPILER_GCC || FEMTOLOG_COMPILER_CLANG
#define FEMTOLOG_PACKED_STRUCT(name) struct __attribute__((packed)) name

#else
#define FEMTOLOG_PACKED_STRUCT(name) struct name

#endif

// Likely/Unlikely branch prediction
#if FEMTOLOG_COMPILER_GCC || FEMTOLOG_COMPILER_CLANG
#define FEMTOLOG_LIKELY(x) __builtin_expect(!!(x), 1)
#define FEMTOLOG_UNLIKELY(x) __builtin_expect(!!(x), 0)

#else
#define FEMTOLOG_LIKELY(x) (x)
#define FEMTOLOG_UNLIKELY(x) (x)

#endif

// ===============
// Path Separators
// ===============

#if FEMTOLOG_IS_WINDOWS
#define FEMTOLOG_DIR_SEPARATOR '\\'
#define FEMTOLOG_DIR_SEPARATOR_STR "\\"
#define FEMTOLOG_PATH_SEPARATOR ';'
#define FEMTOLOG_PATH_SEPARATOR_STR ";"

#else
#define FEMTOLOG_DIR_SEPARATOR '/'
#define FEMTOLOG_DIR_SEPARATOR_STR "/"
#define FEMTOLOG_PATH_SEPARATOR ':'
#define FEMTOLOG_PATH_SEPARATOR_STR ":"

#endif

// =================
// I/O and Threading
// =================

#if FEMTOLOG_IS_WINDOWS
#define FEMTOLOG_USE_WINDOWS_IO 1
#define FEMTOLOG_USE_POSIX_IO 0
#define FEMTOLOG_USE_WINDOWS_THREADS 1
#define FEMTOLOG_USE_POSIX_THREADS 0

#else
#define FEMTOLOG_USE_WINDOWS_IO 0
#define FEMTOLOG_USE_POSIX_IO 1
#define FEMTOLOG_USE_WINDOWS_THREADS 0
#define FEMTOLOG_USE_POSIX_THREADS 1

#endif

#endif  // INCLUDE_FEMTOLOG_BUILD_BUILD_FLAG_H_
