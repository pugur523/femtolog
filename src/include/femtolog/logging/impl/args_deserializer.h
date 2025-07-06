// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_DESERIALIZER_H_
#define INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_DESERIALIZER_H_

#include <cstring>
#include <tuple>
#include <utility>

#include "femtolog/base/serialize_util.h"
#include "femtolog/base/string_registry.h"
#include "fmt/args.h"

namespace femtolog::logging {

template <typename... Args>
class DeserializeDispatcher {
 public:
  inline static DeserializeAndFormatFunction function() {
    return &deserialize_and_format;
  }

 private:
  inline static std::size_t deserialize_and_format(
      fmt::memory_buffer* format_buffer,
      FormatFunction fmt_function,
      StringRegistry* registry,
      const char* data) {
    return deserialize_and_format_impl(format_buffer, fmt_function, registry,
                                       data,
                                       std::index_sequence_for<Args...>{});
  }

  template <std::size_t... I>
  inline static std::size_t deserialize_and_format_impl(
      fmt::memory_buffer* format_buffer,
      FormatFunction fmt_function,
      StringRegistry* registry,
      const char* data,
      std::index_sequence<I...>) {
    if constexpr (sizeof...(Args) == 1) {
      using FirstArg = std::tuple_element_t<0, std::tuple<Args...>>;
      using Decayed = std::decay_t<FirstArg>;

      if constexpr (std::is_trivially_copyable_v<Decayed> &&
                    !is_string_like_v<Decayed>) {
        if constexpr (sizeof(Decayed) <= 8) {
          Decayed value;
          std::memcpy(&value, data, sizeof(value));
          return fmt_function(format_buffer, fmt::make_format_args(value));
        }
      }
    }

    return format_directly(format_buffer, fmt_function, registry, data,
                           std::index_sequence<I...>{});
  }

  template <std::size_t... I>
  inline static std::size_t format_directly(fmt::memory_buffer* format_buffer,
                                            FormatFunction fmt_function,
                                            StringRegistry* registry,
                                            const char* data,
                                            std::index_sequence<I...>) {
    if constexpr (sizeof...(Args) == 1) {
      auto arg0 = read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(
          registry, data, offset_of<0>());
      return fmt_function(format_buffer, fmt::make_format_args(arg0));
    } else if constexpr (sizeof...(Args) == 2) {
      auto arg0 = read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(
          registry, data, offset_of<0>());
      auto arg1 = read_arg<std::tuple_element_t<1, std::tuple<Args...>>>(
          registry, data, offset_of<1>());
      return fmt_function(format_buffer, fmt::make_format_args(arg0, arg1));
    } else if constexpr (sizeof...(Args) == 3) {
      auto arg0 = read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(
          registry, data, offset_of<0>());
      auto arg1 = read_arg<std::tuple_element_t<1, std::tuple<Args...>>>(
          registry, data, offset_of<1>());
      auto arg2 = read_arg<std::tuple_element_t<2, std::tuple<Args...>>>(
          registry, data, offset_of<2>());
      return fmt_function(format_buffer,
                          fmt::make_format_args(arg0, arg1, arg2));
    } else if constexpr (sizeof...(Args) == 4) {
      auto arg0 = read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(
          registry, data, offset_of<0>());
      auto arg1 = read_arg<std::tuple_element_t<1, std::tuple<Args...>>>(
          registry, data, offset_of<1>());
      auto arg2 = read_arg<std::tuple_element_t<2, std::tuple<Args...>>>(
          registry, data, offset_of<2>());
      auto arg3 = read_arg<std::tuple_element_t<3, std::tuple<Args...>>>(
          registry, data, offset_of<3>());
      return fmt_function(format_buffer,
                          fmt::make_format_args(arg0, arg1, arg2, arg3));
    } else {
      auto args =
          std::make_tuple(read_arg<Args>(registry, data, offset_of<I>())...);
      return std::apply(
          [&](auto&... unpacked) {
            return fmt_function(format_buffer,
                                fmt::make_format_args(unpacked...));
          },
          args);
    }
  }

  template <typename T>
  inline static deserialized_arg_type_t<T> read_arg(StringRegistry* registry,
                                                    const char* base,
                                                    std::size_t offset) {
    using Decayed = std::decay_t<T>;
    const char* ptr = base + offset;

    if constexpr (is_string_like_v<Decayed>) {
      StringId id;
      std::memcpy(&id, ptr, sizeof(id));
      std::string_view view = registry->get_string(id);
      return view;
    } else if constexpr (std::is_trivially_copyable_v<Decayed>) {
      Decayed value;
      std::memcpy(&value, ptr, sizeof(value));
      return value;
    } else {
      static_assert(sizeof(Decayed) == 0, "attempted to read unsupported type");
    }
  }

  template <std::size_t I>
  static consteval std::size_t offset_of() {
    constexpr std::array<std::size_t, sizeof...(Args)> offsets = [] {
      std::array<std::size_t, sizeof...(Args)> out{};
      std::size_t offset = 0;
      std::size_t i = 0;
      ((out[i++] = offset, offset += sizeof(serialized_arg_type_t<Args>)), ...);
      return out;
    }();
    return offsets[I];
  }
};

}  // namespace femtolog::logging

#endif  // INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_DESERIALIZER_H_
