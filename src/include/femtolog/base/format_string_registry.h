// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BASE_FORMAT_STRING_REGISTRY_H_
#define INCLUDE_FEMTOLOG_BASE_FORMAT_STRING_REGISTRY_H_

#include <array>
#include <string_view>

#include "core/check.h"
#include "femtolog/base/format_util.h"

namespace femtolog {

constexpr uint16_t kMaxFormatId = 65535;

// Compile-time hash function (FNV-1a variant)
consteval uint16_t hash_format_string(std::string_view str) noexcept {
  uint64_t hash = 0xcbf29ce484222325ull;
  for (char c : str) {
    hash ^= static_cast<uint64_t>(c);
    hash *= 0x100000001b3ull;
  }
  uint16_t id = static_cast<uint16_t>((hash >> 16) ^ (hash & 0xFFFF));
  return id == 0 ? 1 : id;  // avoid 0 (reserved for invalid)
}

namespace internal {

inline std::array<std::string_view, kMaxFormatId> global_table;

inline std::string_view get_format_string(uint16_t id) {
  DCHECK_LE(id, kMaxFormatId) << "invalid format id specified";
  return global_table[id];
}

}  // namespace internal

template <FixedString FormatStr>
consteval inline uint16_t get_format_id() {
  return hash_format_string(FormatStr.view());
}

template <FixedString fixed_str>
inline void register_format() {
  constexpr uint16_t id = get_format_id<fixed_str>();
  constexpr std::string_view view(fixed_str.data, fixed_str.size);
  internal::global_table[id] = view;
}

inline std::string_view get_format_string(uint16_t id) {
  return internal::get_format_string(id);
}

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_BASE_FORMAT_STRING_REGISTRY_H_
