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
      const char* data) {
    return deserialize_and_format_impl(format_buffer, fmt_function, data,
                                       std::index_sequence_for<Args...>{});
  }

  template <std::size_t... I>
  inline static std::size_t deserialize_and_format_impl(
      fmt::memory_buffer* format_buffer,
      FormatFunction fmt_function,
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

    return format_directly(format_buffer, fmt_function, data,
                           std::index_sequence<I...>{});
  }

  template <std::size_t... I>
  inline static std::size_t format_directly(fmt::memory_buffer* format_buffer,
                                            FormatFunction fmt_function,
                                            const char* data,
                                            std::index_sequence<I...>) {
    if constexpr (sizeof...(Args) == 1) {
      const auto [arg0, o0] =
          read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(data, 0);
      return fmt_function(format_buffer, fmt::make_format_args(arg0));
    } else if constexpr (sizeof...(Args) == 2) {
      const auto [arg0, o0] =
          read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(data, 0);
      const auto [arg1, o1] =
          read_arg<std::tuple_element_t<1, std::tuple<Args...>>>(data, o0);
      return fmt_function(format_buffer, fmt::make_format_args(arg0, arg1));
    } else if constexpr (sizeof...(Args) == 3) {
      const auto [arg0, o0] =
          read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(data, 0);
      const auto [arg1, o1] =
          read_arg<std::tuple_element_t<1, std::tuple<Args...>>>(data, o0);
      const auto [arg2, o2] =
          read_arg<std::tuple_element_t<2, std::tuple<Args...>>>(data, o1);
      return fmt_function(format_buffer,
                          fmt::make_format_args(arg0, arg1, arg2));
    } else if constexpr (sizeof...(Args) == 4) {
      const auto [arg0, o0] =
          read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(data, 0);
      const auto [arg1, o1] =
          read_arg<std::tuple_element_t<1, std::tuple<Args...>>>(data, o0);
      const auto [arg2, o2] =
          read_arg<std::tuple_element_t<2, std::tuple<Args...>>>(data, o1);
      const auto [arg3, o3] =
          read_arg<std::tuple_element_t<3, std::tuple<Args...>>>(data, o2);
      return fmt_function(format_buffer,
                          fmt::make_format_args(arg0, arg1, arg2, arg3));
    } else {
      std::size_t offset = 0;
      const auto args = std::make_tuple([&]<typename T>(T) {
        auto [arg, next_offset] = read_arg<T>(data, offset);
        offset = next_offset;
        return arg;
      }(std::tuple_element_t<I, std::tuple<Args...>>{})...);

      return std::apply(
          [&](auto&&... unpacked) {
            return fmt_function(
                format_buffer,
                fmt::make_format_args(
                    std::forward<decltype(unpacked)>(unpacked)...));
          },
          args);
    }
  }

  template <typename T>
  inline static std::pair<deserialized_arg_type_t<T>, std::size_t> read_arg(
      const char* base,
      std::size_t offset) {
    using Decayed = std::decay_t<T>;
    const char* ptr = base + offset;

    if constexpr (is_string_like_v<Decayed>) {
      std::size_t str_len;
      std::memcpy(&str_len, ptr, sizeof(str_len));
      ptr += sizeof(str_len);

      std::string str;
      if (str_len > 0) {
        str.resize(str_len);
        std::memcpy(str.data(), ptr, str_len);
      }
      return {str, offset + sizeof(str_len) + str_len};
    } else if constexpr (std::is_trivially_copyable_v<Decayed>) {
      Decayed value;
      std::memcpy(&value, ptr, sizeof(value));
      return {value, offset + sizeof(value)};
    } else {
      static_assert(sizeof(Decayed) == 0, "attempted to read unsupported type");
      return {};
    }
  }
};

}  // namespace femtolog::logging

#endif  // INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_DESERIALIZER_H_
