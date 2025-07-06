// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BASE_STRING_REGISTRY_H_
#define INCLUDE_FEMTOLOG_BASE_STRING_REGISTRY_H_

#include <array>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

#include "femtolog/base/format_util.h"
#include "femtolog/build/build_flag.h"
#include "femtolog/core/check.h"

namespace femtolog {

constexpr uint16_t kUint16Max = std::numeric_limits<uint16_t>::max();

using StringId = uint16_t;

// Assign uint16 max value for literal string log
constexpr StringId kLiteralLogStringId = kUint16Max;

// Compiletime hash using FNV-1a variant
consteval uint16_t hash_string(std::string_view str) noexcept {
  uint64_t hash = 0xcbf29ce484222325ull;
  for (char c : str) {
    hash ^= static_cast<uint64_t>(c);
    hash *= 0x100000001b3ull;
  }
  return static_cast<StringId>((hash >> 16) ^ (hash & 0xFFFF));
}

// Runtime hash using raw char pointer
inline StringId hash_string_from_pointer(const char* ptr) noexcept {
  uintptr_t raw = reinterpret_cast<uintptr_t>(ptr);
  return static_cast<StringId>((raw >> 3) ^ (raw & 0xFFFF));
}

class StringRegistry {
 public:
  StringRegistry()
      : table_(kUint16Max + 1),
        arena_buffer_(std::make_unique<char[]>(kInitialArenaSize)) {
    // Construct all string_view as empty string
    table_.resize(table_.capacity());
  }

  ~StringRegistry() = default;

  StringRegistry(const StringRegistry&) = delete;
  StringRegistry& operator=(const StringRegistry&) = delete;

  StringRegistry(StringRegistry&&) noexcept = default;
  StringRegistry& operator=(StringRegistry&&) noexcept = default;

  template <FixedString fixed_str>
  inline void register_string() {
    constexpr StringId id = get_string_id<fixed_str>();
    constexpr std::string_view view(fixed_str.data, fixed_str.size);
    set(id, view);
  }

  void register_string(StringId id, std::string_view view) { set(id, view); }

  void register_string_arena(StringId id, std::string_view view) {
    FEMTOLOG_DCHECK_LE(arena_size_ + view.size(), arena_capacity_);

    if (!table_[id].empty()) [[likely]] {
      return;
    }

    char* dest = arena_buffer_.get() + arena_size_;
    // include null termination
    std::size_t len = view.size();
    std::memcpy(dest, view.data(), len);

    std::string_view stored(dest, len);
    arena_size_ += len;
    set(id, stored);
  }

  inline std::string_view get_string(StringId id) const { return table_[id]; }

  template <FixedString fixed_str>
  static consteval StringId get_string_id() {
    constexpr StringId id = hash_string(fixed_str.view());
    return id == kLiteralLogStringId ? kLiteralLogStringId - 1 : id;
  }

  static consteval StringId get_string_id(std::string_view view) {
    StringId id = hash_string(view);
    return id == kLiteralLogStringId ? kLiteralLogStringId - 1 : id;
  }

 private:
  void set(StringId id, std::string_view view) {
    FEMTOLOG_DCHECK_LT(id, kUint16Max);
    table_[id] = view;
  }

  static constexpr std::size_t kInitialArenaSize = 1024 * 1024;  // 1MiB

  std::vector<std::string_view> table_;
  std::unique_ptr<char[]> arena_buffer_;
  std::size_t arena_size_ = 0;
#if FEMTOLOG_IS_DEBUG
  std::size_t arena_capacity_ = kInitialArenaSize;
#endif
};

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_BASE_STRING_REGISTRY_H_
