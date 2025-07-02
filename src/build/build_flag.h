// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef BUILD_BUILD_FLAG_H_
#define BUILD_BUILD_FLAG_H_

// ==================
// Platform Detection
// ==================

#define IS_WINDOWS 0
#define IS_UNIX 0
#define IS_MAC 0
#define IS_IOS 0
#define IS_ANDROID 0
#define IS_LINUX 0
#define IS_BSD 0
#define IS_CYGWIN 0
#define IS_SOLARIS 0

// Override based on actual platform
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
#undef IS_WINDOWS
#define IS_WINDOWS 1

#elif defined(__APPLE__)
#include <TargetConditionals.h>
#undef IS_UNIX
#define IS_UNIX 1

#if TARGET_OS_MAC
#undef IS_MAC
#define IS_MAC 1

#elif TARGET_OS_IPHONE
#undef IS_IOS
#define IS_IOS 1

#else
#undef IS_MAC
#define IS_MAC 1

#endif

#elif defined(__linux__) || defined(__linux) || defined(linux)
#undef IS_UNIX
#define IS_UNIX 1

#if defined(__ANDROID__)
#undef IS_ANDROID
#define IS_ANDROID 1

#else
#undef IS_LINUX
#define IS_LINUX 1

#endif

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__DragonFly__)
#undef IS_UNIX
#undef IS_BSD
#define IS_UNIX 1
#define IS_BSD 1

#elif defined(__CYGWIN__)
#undef IS_UNIX
#undef IS_CYGWIN
#define IS_UNIX 1
#define IS_CYGWIN 1

#elif defined(__sun) && defined(__SVR4)
#undef IS_UNIX
#undef IS_SOLARIS
#define IS_UNIX 1
#define IS_SOLARIS 1

#else
#error "Unsupported platform detected."

#endif

// ==================
// Compiler Detection
// ==================

#if defined(_MSC_VER)
#define COMPILER_MSVC 1
#define COMPILER_GCC 0
#define COMPILER_CLANG 0
#define COMPILER_INTEL 0
#define COMPILER_MINGW 0

#elif defined(__clang__)
#define COMPILER_MSVC 0
#define COMPILER_GCC 0
#define COMPILER_CLANG 1
#define COMPILER_INTEL 0
#define COMPILER_MINGW 0

#elif defined(__INTEL_COMPILER) || defined(__ICC)
#define COMPILER_MSVC 0
#define COMPILER_GCC 0
#define COMPILER_CLANG 0
#define COMPILER_INTEL 1
#define COMPILER_MINGW 0

#elif defined(__MINGW32__) || defined(__MINGW64__)
#define COMPILER_MSVC 0
#define COMPILER_GCC 0
#define COMPILER_CLANG 0
#define COMPILER_INTEL 0
#define COMPILER_MINGW 1

#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_MSVC 0
#define COMPILER_GCC 1
#define COMPILER_CLANG 0
#define COMPILER_INTEL 0
#define COMPILER_MINGW 0

#else
#error "Unsupported compiler detected."

#endif

// ======================
// Architecture Detection
// ======================

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#define ARCH_X64 1
#define ARCH_X86 0
#define ARCH_ARM 0
#define ARCH_ARM64 0
#define ARCH_MIPS 0
#define ARCH_PPC 0
#define ARCH_RISCV 0

#elif defined(__i386) || defined(_M_IX86) || defined(__i386__)
#define ARCH_X64 0
#define ARCH_X86 1
#define ARCH_ARM 0
#define ARCH_ARM64 0
#define ARCH_MIPS 0
#define ARCH_PPC 0
#define ARCH_RISCV 0

#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCH_X64 0
#define ARCH_X86 0
#define ARCH_ARM 0
#define ARCH_ARM64 1
#define ARCH_MIPS 0
#define ARCH_PPC 0
#define ARCH_RISCV 0

#elif defined(__arm__) || defined(_M_ARM)
#define ARCH_X64 0
#define ARCH_X86 0
#define ARCH_ARM 1
#define ARCH_ARM64 0
#define ARCH_MIPS 0
#define ARCH_PPC 0
#define ARCH_RISCV 0

#elif defined(__mips__) || defined(__mips) || defined(__MIPS__)
#define ARCH_X64 0
#define ARCH_X86 0
#define ARCH_ARM 0
#define ARCH_ARM64 0
#define ARCH_MIPS 1
#define ARCH_PPC 0
#define ARCH_RISCV 0

#elif defined(__powerpc__) || defined(__powerpc) || defined(__ppc__) || \
    defined(__PPC__)
#define ARCH_X64 0
#define ARCH_X86 0
#define ARCH_ARM 0
#define ARCH_ARM64 0
#define ARCH_MIPS 0
#define ARCH_PPC 1
#define ARCH_RISCV 0

#elif defined(__riscv)
#define ARCH_X64 0
#define ARCH_X86 0
#define ARCH_ARM 0
#define ARCH_ARM64 0
#define ARCH_MIPS 0
#define ARCH_PPC 0
#define ARCH_RISCV 1

#else
#define ARCH_X64 0
#define ARCH_X86 0
#define ARCH_ARM 0
#define ARCH_ARM64 0
#define ARCH_MIPS 0
#define ARCH_PPC 0
#define ARCH_RISCV 0

#error "Unknown architecture detected"

#endif

// Pointer size detection
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__) || \
    defined(__powerpc64__) || defined(__mips64) ||                    \
    (defined(__riscv) && __riscv_xlen == 64)
#define ARCH_64BIT 1
#define ARCH_32BIT 0

#else
#define ARCH_64BIT 0
#define ARCH_32BIT 1

#endif

// ====================
// Endianness Detection
// ====================

#if defined(__BYTE_ORDER__)

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define IS_BIG_ENDIAN 1
#define IS_LITTLE_ENDIAN 0

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IS_BIG_ENDIAN 0
#define IS_LITTLE_ENDIAN 1

#else
#error "Unknown byte order"

#endif

#elif defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || defined(_MIPSEB) || defined(__MIPSEB) ||         \
    defined(__MIPSEB__)
#define IS_BIG_ENDIAN 1
#define IS_LITTLE_ENDIAN 0

#elif defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) ||                 \
    defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL) || \
    defined(__MIPSEL) || defined(__MIPSEL__)
#define IS_BIG_ENDIAN 0
#define IS_LITTLE_ENDIAN 1

#else
// Default to little endian for most modern systems
#define IS_BIG_ENDIAN 0
#define IS_LITTLE_ENDIAN 1
#pragma message("Endianness detection failed, defaulting to little endian")

#endif

// ===================
// Build Configuration
// ===================

#ifdef NDEBUG
#define IS_DEBUG 0
#define IS_RELEASE 1

#else
#define IS_DEBUG 1
#define IS_RELEASE 0

#endif

#if FEMTOLOG_ENABLE_XRAY
#define XRAY_FN [[clang::xray_always_instrument]]
#define XRAY_FN_LOG [[clang::xray_always_instrument, clang::xray_log_args(1)]]

#else
#define XRAY_FN
#define XRAY_FN_LOG

#endif

// ==========================
// Compiler Version Detection
// ==========================

#if COMPILER_MSVC
#define COMPILER_VERSION _MSC_VER

#elif COMPILER_GCC
#define COMPILER_VERSION \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#elif COMPILER_CLANG
#define COMPILER_VERSION \
  (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)

#elif COMPILER_INTEL
#define COMPILER_VERSION __INTEL_COMPILER

#else
#define COMPILER_VERSION 0

#endif

// ===================
// Function Attributes
// ===================

// Inline
#if COMPILER_MSVC
#define FORCE_INLINE __forceinline
#define INLINE __inline

#elif COMPILER_GCC || COMPILER_CLANG
#define FORCE_INLINE inline __attribute__((always_inline))
#define INLINE inline

#else
#define FORCE_INLINE inline
#define INLINE inline

#endif

// No inline
#if COMPILER_MSVC
#define NO_INLINE __declspec(noinline)

#elif COMPILER_GCC || COMPILER_CLANG
#define NO_INLINE __attribute__((noinline))

#else
#define NO_INLINE

#endif

// Memory alignment
#if COMPILER_MSVC
#define ALIGN(x) __declspec(align(x))

#elif COMPILER_GCC || COMPILER_CLANG
#define ALIGN(x) __attribute__((aligned(x)))

#else
#define ALIGN(x)

#endif

// Packed structures
#if COMPILER_MSVC
#define PACKED_STRUCT(name) \
  __pragma(pack(push, 1)) struct name __pragma(pack(pop))

#elif COMPILER_GCC || COMPILER_CLANG
#define PACKED_STRUCT(name) struct __attribute__((packed)) name

#else
#define PACKED_STRUCT(name) struct name

#endif

// Likely/Unlikely branch prediction
#if COMPILER_GCC || COMPILER_CLANG
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)

#endif

// ===============
// Path Separators
// ===============

#if IS_WINDOWS
#define DIR_SEPARATOR '\\'
#define DIR_SEPARATOR_STR "\\"
#define PATH_SEPARATOR ';'
#define PATH_SEPARATOR_STR ";"

#else
#define DIR_SEPARATOR '/'
#define DIR_SEPARATOR_STR "/"
#define PATH_SEPARATOR ':'
#define PATH_SEPARATOR_STR ":"

#endif

// =================
// I/O and Threading
// =================

#if IS_WINDOWS
#define USE_WINDOWS_IO 1
#define USE_POSIX_IO 0
#define USE_WINDOWS_THREADS 1
#define USE_POSIX_THREADS 0

#else
#define USE_WINDOWS_IO 0
#define USE_POSIX_IO 1
#define USE_WINDOWS_THREADS 0
#define USE_POSIX_THREADS 1

#endif

#endif  // BUILD_BUILD_FLAG_H_
