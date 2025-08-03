// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_SERIALIZER_H_
#define INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_SERIALIZER_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>
#include <string>
#include <string_view>

#include "femtolog/base/format_util.h"
#include "femtolog/base/serialize_util.h"
#include "femtolog/base/string_registry.h"
#include "femtolog/build/build_flag.h"
#include "femtolog/core/base/memory_util.h"
#include "femtolog/core/check.h"
#include "femtolog/logging/impl/args_deserializer.h"

namespace femtolog::logging {

template <typename T>
inline void write_arg(StringRegistry* registry, char*& pos, const T& value) {
  using Decayed = std::decay_t<T>;

  if constexpr (is_string_like_v<Decayed>) {
    std::string_view view;
    if constexpr (is_char_array_v<Decayed>) {
      if (value == nullptr) [[unlikely]] {
        view = "nullptr";
      } else {
        view = value;
      }
    } else {
      view = value;
    }

    StringId id = hash_string_from_pointer(view.data());
    registry->register_string_arena(id, view);
    std::memcpy(pos, &id, sizeof(id));
    pos += sizeof(id);
  } else if constexpr (std::is_trivially_copyable_v<Decayed>) {
    std::memcpy(pos, &value, sizeof(Decayed));
    pos += sizeof(Decayed);
  } else {
    static_assert(sizeof(Decayed) == 0,
                  "attempted to serialize unsupported format argument type.");
  }
}

template <typename... Args>
consteval std::size_t calculate_serialized_size() {
  std::size_t total = sizeof(SerializedArgsHeader);
  auto add_arg_size = []<typename T>() constexpr -> std::size_t {
    using Decayed = std::decay_t<T>;
    if constexpr (is_string_like_v<Decayed>) {
      return sizeof(StringId);
    } else if constexpr (std::is_trivially_copyable_v<Decayed>) {
      return sizeof(Decayed);
    } else {
      static_assert(sizeof(Decayed) == 0,
                    "attempted to write unsupported type\n:"
                    "currently only supporting string like types and trivially "
                    "copyable types");
    }
  };

  ((total += add_arg_size.template operator()<Args>()), ...);
  return total;
}

template <std::size_t Capacity = 512>
class ArgsSerializer {
 public:
  ArgsSerializer() = default;
  ~ArgsSerializer() = default;

  ArgsSerializer(const ArgsSerializer&) = delete;
  ArgsSerializer& operator=(const ArgsSerializer&) = delete;

  ArgsSerializer(ArgsSerializer&&) = default;
  ArgsSerializer& operator=(ArgsSerializer&&) = default;

  inline const SerializedArgs<Capacity>& args() const { return args_; }
  inline SerializedArgs<Capacity>& args() { return args_; }

  // for 0 args: we don't use this
  template <FixedString fmt>
  SerializedArgs<Capacity>& serialize(StringRegistry*) = delete;

  template <FixedString fmt, typename... Args>
  [[nodiscard, gnu::hot]] constexpr SerializedArgs<Capacity>& serialize(
      StringRegistry* registry,
      Args&&... args) {
    constexpr std::size_t total_size = calculate_serialized_size<Args...>();
    static_assert(Capacity >= total_size, "Buffer too small for arguments");

    FormatFunction format_function_ptr =
        FormatDispatcher<fmt, std::decay_t<Args>...>::function();
    DeserializeAndFormatFunction deserialize_function_ptr =
        DeserializeDispatcher<std::decay_t<Args>...>::function();

    // header
    const SerializedArgsHeader header(format_function_ptr,
                                      deserialize_function_ptr);
    char* pos = args_.data();
    std::memcpy(pos, &header, sizeof(header));
    pos += sizeof(header);

    (write_arg(registry, pos, args), ...);

    args_.resize(total_size);
    return args_;
  }

 private:
  alignas(core::kCacheSize) SerializedArgs<Capacity> args_;
};

// Type aliases
using DefaultSerializer = ArgsSerializer<512>;
using SmallSerializer = ArgsSerializer<256>;
using LargeSerializer = ArgsSerializer<1024>;

using DefaultSerializedArgs = SerializedArgs<512>;
using SmallSerializedArgs = SerializedArgs<256>;
using LargeSerializedArgs = SerializedArgs<1024>;

}  // namespace femtolog::logging

#endif  // INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_SERIALIZER_H_
