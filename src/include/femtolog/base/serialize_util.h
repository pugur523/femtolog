// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_BASE_SERIALIZE_UTIL_H_
#define INCLUDE_FEMTOLOG_BASE_SERIALIZE_UTIL_H_

#include <cstddef>
#include <cstdint>
#include <new>

#include "femtolog/base/format_util.h"
#include "femtolog/base/string_registry.h"
#include "femtolog/core/base/memory_util.h"
#include "femtolog/core/check.h"

namespace femtolog {

using DeserializeAndFormatFunction = std::size_t (*)(fmt::memory_buffer*,
                                                     FormatFunction,
                                                     StringRegistry*,
                                                     const char*);

struct SerializedArgsHeader {
  FormatFunction format_func;
  DeserializeAndFormatFunction deserialize_and_format_func;

  constexpr SerializedArgsHeader(
      FormatFunction format_func_ptr,
      DeserializeAndFormatFunction deserialize_func_ptr)
      : format_func(format_func_ptr),
        deserialize_and_format_func(deserialize_func_ptr) {}
};

template <std::size_t Capacity = 4096>
class SerializedArgs {
 public:
  inline char* data() noexcept { return buffer_; }
  inline const char* data() const noexcept { return buffer_; }

  inline void resize(std::size_t size) noexcept {
    FEMTOLOG_DCHECK_LE(size, Capacity);
    size_ = size;
  }

  inline void clear() noexcept { size_ = 0; }

  inline std::size_t size() const noexcept { return size_; }
  inline consteval std::size_t capacity() const noexcept { return Capacity; }

 private:
  alignas(core::kCacheSize) char buffer_[Capacity];
  std::size_t size_ = 0;
};

template <typename T>
struct IsFixedStringTrait : std::false_type {};

template <std::size_t N>
struct IsFixedStringTrait<const char (&)[N]> : std::true_type {};

template <typename T>
struct ArgTypeInfo {
  using Decayed = std::decay_t<T>;

  static constexpr bool is_char_array =
      std::is_array_v<Decayed> &&
      std::is_same_v<std::remove_extent_t<Decayed>, char>;

  static constexpr bool is_static_string = IsFixedStringTrait<Decayed>::value;

  static constexpr bool is_dynamic_string =
      (std::is_convertible_v<Decayed, std::string_view> ||
       std::is_same_v<Decayed, std::string_view> || is_char_array) &&
      !is_static_string;

  static constexpr bool is_string_like = is_dynamic_string || is_static_string;

  static constexpr bool is_serializeable =
      is_string_like || std::is_trivially_copyable_v<Decayed>;

  ArgTypeInfo() = delete;
};

template <typename T>
inline constexpr bool is_char_array_v = ArgTypeInfo<T>::is_char_array;
template <typename T>
inline constexpr bool is_static_string_v = ArgTypeInfo<T>::is_static_string;
template <typename T>
inline constexpr bool is_dynamic_string_v = ArgTypeInfo<T>::is_dynamic_string;
template <typename T>
inline constexpr bool is_string_like_v = ArgTypeInfo<T>::is_string_like;
template <typename T>
inline constexpr bool is_serializeable_v = ArgTypeInfo<T>::is_serializeable;

template <typename T>
struct SerializedArgType {
  using type =
      std::conditional_t<is_string_like_v<T>, StringId, std::decay_t<T>>;
};
template <typename T>
using serialized_arg_type_t = typename SerializedArgType<T>::type;

template <typename T>
struct DeserializedArgType {
  using type = std::
      conditional_t<is_string_like_v<T>, std::string_view, std::decay_t<T>>;
};
template <typename T>
using deserialized_arg_type_t = typename DeserializedArgType<T>::type;

}  // namespace femtolog

#endif  // INCLUDE_FEMTOLOG_BASE_SERIALIZE_UTIL_H_
