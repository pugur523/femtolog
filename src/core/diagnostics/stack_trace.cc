// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/core/diagnostics/stack_trace.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>

#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/file_util.h"
#include "femtolog/core/base/string_util.h"
#include "femtolog/core/diagnostics/stack_trace_entry.h"

#if FEMTOLOG_IS_WINDOWS
// clang-format off
#include <windows.h>
#include <dbghelp.h>
// clang-format on
#elif FEMTOLOG_IS_UNIX
#include <cxxabi.h>
#include <dlfcn.h>
#include <unistd.h>
#endif

#if FEMTOLOG_ENABLE_LLVM_UNWIND && !FEMTOLOG_COMPILER_MSVC
#include <libunwind.h>
#elif FEMTOLOG_IS_UNIX
#include <execinfo.h>
#endif

namespace femtolog::core {

namespace {

void stack_trace_entries_to_buffer(
    const StackTraceEntry entries[kPlatformMaxFrames],
    std::size_t count,
    char* buffer,
    std::size_t buffer_size) {
  if (!buffer || buffer_size == 0) {
    return;
  }

  char line_buffer[kLineBufferSize];
  std::size_t written = 0;

  for (std::size_t i = 0; i < count && written < buffer_size - 1; i++) {
    entries[i].to_string(line_buffer, sizeof(line_buffer));
    std::size_t line_len = safe_strlen(line_buffer);

    if (written + line_len + 1 < buffer_size) {
      char* cursor = buffer + written;
      std::size_t raw_written = write_raw(cursor, line_buffer, line_len);
      written += raw_written;

      if (written < buffer_size - 1) {
        buffer[written] = '\n';
        written++;
      }
    } else {
      break;
    }
  }
  buffer[written] = '\0';
}

#if FEMTOLOG_IS_UNIX

const char* demangle_symbol_safe(const char* mangled_name,
                                 char* demangled_buffer,
                                 std::size_t demangled_len) {
  if (!mangled_name || !demangled_buffer || demangled_len == 0) {
    return mangled_name;
  }

  int status = 0;

  // Try to demangle - __cxa_demangle may allocate internally but we control the
  // output buffer
  char* demangled = abi::__cxa_demangle(mangled_name, demangled_buffer,
                                        &demangled_len, &status);

  // If demangling succeeded and the result fits in our buffer
  if (demangled && status == 0 && demangled == demangled_buffer) {
    return demangled_buffer;
  }

  if (demangled && status == 0) {
    write_raw(demangled_buffer, demangled, demangled_len);
    free(demangled);  // __cxa_demangle allocated this
    return demangled_buffer;
  }

  return mangled_name;
}

#endif  // FEMTOLOG_IS_UNIX

bool is_valid_address(uintptr_t addr) {
  if (addr == 0 || addr < 0x1000) {
    return false;
  }

// Basic sanity check for user space addresses
// This is a conservative check that should work on most systems
#if FEMTOLOG_ARCH_X64
  // 64-bit: user space typically below 0x800000000000
  if (addr >= 0x800000000000ULL) {
    return false;
  }
#elif FEMTOLOG_ARCH_X86
  // 32-bit: user space typically below 0xC0000000
  if (addr >= 0xC0000000UL) {
    return false;
  }
#endif

  return true;
}

}  // namespace

std::size_t collect_stack_trace(StackTraceEntry out[kPlatformMaxFrames],
                                bool use_index,
                                std::size_t first_frame,
                                std::size_t max_frames) {
  if (!out) {
    return 0;
  }

  max_frames = std::min(max_frames, kPlatformMaxFrames);
  first_frame = std::min(first_frame, max_frames);

  if (first_frame >= max_frames) {
    return 0;
  }

#if FEMTOLOG_ENABLE_LLVM_UNWIND
  unw_context_t context;
  if (unw_getcontext(&context) != 0) {
    return 0;
  }

  unw_cursor_t cursor;
  if (unw_init_local(&cursor, &context) != 0) {
    return 0;
  }

  char symbol_buffer[kSymbolBufferSize];
  char address_buffer[kAddressStrLength];
  char demangled_buffer[kDemangledBufferSize];

  std::size_t frame_index = 0;
  std::size_t collected_count = 0;

  do {
    if (frame_index < first_frame) {
      ++frame_index;
      continue;
    }

    if (collected_count >= max_frames) {
      break;
    }

    unw_word_t instruction_pointer = 0;
    if (unw_get_reg(&cursor, UNW_REG_IP, &instruction_pointer) != 0) {
      ++frame_index;
      continue;
    }

    // Validate address before processing
    if (!is_valid_address(static_cast<uintptr_t>(instruction_pointer))) {
      ++frame_index;
      continue;
    }

    StackTraceEntry& entry = out[collected_count];
    entry.index = use_index ? (frame_index - first_frame) : 0;
    entry.line = 0;
    entry.offset = 0;
    entry.use_index = use_index;

    // Format address
    format_address_safe(static_cast<uintptr_t>(instruction_pointer),
                        address_buffer, sizeof(address_buffer));
    char* address_cursor = entry.address.data();
    write_raw(address_cursor, address_buffer, sizeof(address_buffer));

    unw_word_t symbol_offset = 0;
    const char* function_name = "";

    int symbol_result = unw_get_proc_name(
        &cursor, symbol_buffer, sizeof(symbol_buffer), &symbol_offset);
    if (symbol_result == 0) {
      function_name = demangle_symbol_safe(symbol_buffer, demangled_buffer,
                                           sizeof(demangled_buffer));
      entry.offset = static_cast<std::size_t>(symbol_offset);
    } else {
      Dl_info dl_info;
      std::memset(&dl_info, 0, sizeof(dl_info));

      if (dladdr(std::bit_cast<const void*>(instruction_pointer), &dl_info)) {
        if (dl_info.dli_sname) {
          char* symbol_cursor = symbol_buffer;
          write_raw(symbol_cursor, dl_info.dli_sname, kSymbolBufferSize);
          function_name = demangle_symbol_safe(symbol_buffer, demangled_buffer,
                                               sizeof(demangled_buffer));

          if (dl_info.dli_saddr) {
            uintptr_t symbol_base =
                reinterpret_cast<uintptr_t>(dl_info.dli_saddr);
            uintptr_t current_addr =
                static_cast<uintptr_t>(instruction_pointer);
            if (current_addr >= symbol_base) {
              entry.offset = current_addr - symbol_base;
            }
          }
        }
      }
    }

    char* function_cursor = entry.function.data();
    write_raw(function_cursor, function_name, safe_strlen(function_name));

    const char* file_name = "";
    Dl_info dl_info;
    std::memset(&dl_info, 0, sizeof(dl_info));

    if (dladdr(std::bit_cast<const void*>(instruction_pointer), &dl_info)) {
      if (dl_info.dli_fname) {
        file_name = dl_info.dli_fname;
      }
    }

    char* file_cursor = entry.file.data();
    write_raw(file_cursor, file_name, kFileStrLength);

    collected_count++;
    frame_index++;
  } while (unw_step(&cursor) > 0);

  return collected_count;

#elif FEMTOLOG_IS_UNIX
  void* stack[kPlatformMaxFrames];
  int frames = backtrace(stack, static_cast<int>(max_frames));
  char address_buf[kAddressStrLength];
  char demangled_buf[kDemangledBufferSize];
  char symbol_buf[kSymbolBufferSize];
  std::size_t collected_count = 0;

  for (std::size_t i = first_frame; i < static_cast<std::size_t>(frames); i++) {
    uintptr_t current_address = reinterpret_cast<uintptr_t>(stack[i]);

    if (!is_valid_address(current_address)) {
      continue;
    }

    StackTraceEntry& entry = out[collected_count];
    entry.index = use_index ? (i - first_frame) : 0;
    entry.line = 0;
    entry.use_index = use_index;

    format_address_safe(current_address, address_buf, sizeof(address_buf));
    char* address_cursor = entry.address.data();
    write_raw(address_cursor, address_buf, sizeof(address_buf));

    const char* function_name = "";
    const char* file_name = "";
    std::size_t offset = 0;

    Dl_info dl_info;
    std::memset(&dl_info, 0, sizeof(dl_info));

    if (dladdr(std::bit_cast<const void*>(current_address), &dl_info)) {
      if (dl_info.dli_sname) {
        char* symbol_cursor = symbol_buf;
        write_raw(symbol_cursor, dl_info.dli_sname, sizeof(symbol_buf));
        function_name = demangle_symbol_safe(symbol_buf, demangled_buf,
                                             kDemangledBufferSize);
      }
      if (dl_info.dli_fname) {
        file_name = dl_info.dli_fname;
      }
      if (dl_info.dli_saddr) {
        std::uintptr_t symbol_base =
            reinterpret_cast<std::uintptr_t>(dl_info.dli_saddr);
        if (current_address >= symbol_base) {
          offset = current_address - symbol_base;
        }
      }
    }
    entry.offset = offset;

    char* function_cursor = entry.function.data();
    char* file_cursor = entry.file.data();
    write_raw(function_cursor, function_name, kFunctionStrLength);
    write_raw(file_cursor, file_name, kFileStrLength);

    collected_count++;
  }

  return collected_count;
#elif FEMTOLOG_IS_WINDOWS
  HANDLE process = GetCurrentProcess();

  void* stack[kPlatformMaxFrames * 2];
  WORD raw_frames = CaptureStackBackTrace(
      0, static_cast<DWORD>(first_frame + max_frames), stack, nullptr);
  WORD frames = std::min<WORD>(raw_frames - first_frame, max_frames);

  constexpr int kMaxNameLen = 256;
  // alignas(SYMBOL_INFO) char symbol_buffer[sizeof(SYMBOL_INFO) +
  // kMaxNameLen];
  char symbol_buffer[sizeof(SYMBOL_INFO) + kMaxNameLen * sizeof(TCHAR)];
  SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbol_buffer);
  std::memset(symbol, 0, sizeof(symbol_buffer));

  symbol->MaxNameLen = kMaxNameLen;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  IMAGEHLP_LINE64 line;
  line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
  DWORD displacement;

  char address_buf[32];
  std::size_t count = 0;

  for (WORD i = 0; i < frames; i++) {
    DWORD64 current_address = reinterpret_cast<DWORD64>(stack[i + first_frame]);

    // Skip invalid addresses
    if (!is_valid_address(current_address)) {
      continue;
    }

    format_address_safe(current_address, address_buf, sizeof(address_buf));
    const char* addr_str = address_buf;

    const char* function = "";
    const char* file = "";
    int line_number = 0;

    if (SymFromAddr(process, current_address, nullptr, symbol)) {
      function = symbol->Name;
    }

    if (SymGetLineFromAddr64(process, current_address, &displacement, &line)) {
      file = line.FileName;
      line_number = line.LineNumber;
    }

    StackTraceEntry entry;
    entry.index = use_index ? i : 0;
    char* address_cursor = entry.address.data();
    char* function_cursor = entry.function.data();
    char* file_cursor = entry.file.data();
    write_raw(address_cursor, addr_str, kAddressStrLength);
    write_raw(function_cursor, function, kFunctionStrLength);
    write_raw(file_cursor, file, kFileStrLength);
    entry.line = line_number;
    entry.offset = 0;
    entry.use_index = use_index;
    out[count++] = entry;
  }

  return count;
#endif
}

std::string stack_trace_from_current_context(bool use_index,
                                             std::size_t first_frame,
                                             std::size_t max_frames) {
  constexpr const std::size_t kBufSize = 4096;
  char buf[kBufSize];
  stack_trace_from_current_context(buf, kBufSize, use_index, first_frame + 1,
                                   max_frames);
  return std::string(buf);
}

void stack_trace_from_current_context(char* buffer,
                                      std::size_t buffer_size,
                                      bool use_index,
                                      std::size_t first_frame,
                                      std::size_t max_frames) {
  StackTraceEntry entries[kPlatformMaxFrames];
  std::size_t count =
      collect_stack_trace(entries, use_index, first_frame, max_frames);
  stack_trace_entries_to_buffer(entries, count, buffer, buffer_size);
}

#if FEMTOLOG_IS_WINDOWS

namespace {

struct WindowsStackTraceHandler {
  WindowsStackTraceHandler() {
    DWORD sym_options = SymGetOptions();
    sym_options |= SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES;
    sym_options |= SYMOPT_FAIL_CRITICAL_ERRORS;  // Don't show error dialogs
    sym_options &= ~SYMOPT_NO_PROMPTS;           // Disable prompts
    SymSetOptions(sym_options);
    SymInitialize(GetCurrentProcess(), exe_dir().c_str(), TRUE);
  }

  ~WindowsStackTraceHandler() { SymCleanup(GetCurrentProcess()); }
};

}  // namespace

#endif  // FEMTOLOG_IS_WINDOWS

void register_stack_trace_handler() {
#if FEMTOLOG_IS_WINDOWS
  static const WindowsStackTraceHandler handler;
#endif
}

}  // namespace femtolog::core
