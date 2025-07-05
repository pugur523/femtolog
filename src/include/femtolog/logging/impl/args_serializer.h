// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_SERIALIZER_H_
#define INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_SERIALIZER_H_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "femtolog/base/string_registry.h"
#include "femtolog/build/build_flag.h"
#include "femtolog/core/check.h"

namespace femtolog::logging {

enum class ArgType : uint8_t {
  kInt32 = 0,
  kInt64 = 1,
  kUint32 = 2,
  kUint64 = 3,
  kFloat = 4,
  kDouble = 5,
  kBool = 6,
  kChar = 7,
  kString = 8,
  kFixedString = 9,
  kPointer = 10,
};

#pragma pack(push, 1)
struct ArgHeader {
  ArgType type;
  uint16_t size;

  constexpr ArgHeader(ArgType t, uint16_t s) : type(t), size(s) {}
};
struct SerializedArgsHeader {
  uint32_t total_size = 0;
  uint16_t arg_count = 0;
  uint16_t reserved = 0;

  constexpr SerializedArgsHeader(uint32_t size, uint16_t count)
      : total_size(size), arg_count(count) {}
};
#pragma pack(pop)

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
  alignas(std::hardware_destructive_interference_size) char buffer_[Capacity];
  std::size_t size_ = 0;
};

template <typename T>
struct is_fixed_string_trait : std::false_type {};

template <std::size_t N>
struct is_fixed_string_trait<FixedString<N>> : std::true_type {};

template <std::size_t N>
struct is_fixed_string_trait<const char (&)[N]> : std::true_type {};

template <typename T>
struct ArgTypeInfo {
  using DecayT = std::decay_t<T>;

  static constexpr bool is_char_array =
      std::is_array_v<DecayT> &&
      std::is_same_v<std::remove_extent_t<DecayT>, char>;

  static constexpr bool is_fixed_string = is_fixed_string_trait<DecayT>::value;

  static constexpr bool is_dynamic_string =
      (std::is_convertible_v<DecayT, std::string_view> ||
       std::is_same_v<DecayT, std::string_view> || is_char_array) &&
      !is_fixed_string;

  static constexpr bool is_string_like = is_dynamic_string || is_fixed_string;

  static constexpr ArgType type = []() constexpr {
    if constexpr (std::is_same_v<DecayT, int32_t> ||
                  std::is_same_v<DecayT, int>) {
      return ArgType::kInt32;
    } else if constexpr (std::is_same_v<DecayT, int64_t> ||    // NOLINT
                         std::is_same_v<DecayT, long> ||       // NOLINT
                         std::is_same_v<DecayT, long long>) {  // NOLINT
      return ArgType::kInt64;
    } else if constexpr (std::is_same_v<DecayT, uint32_t> ||  // NOLINT
                         std::is_same_v<DecayT, unsigned int>) {
      return ArgType::kUint32;
    } else if constexpr (std::is_same_v<DecayT, uint64_t> ||       // NOLINT
                         std::is_same_v<DecayT, unsigned long> ||  // NOLINT
                         std::is_same_v<DecayT,
                                        unsigned long long>) {  // NOLINT
      return ArgType::kUint64;
    } else if constexpr (std::is_same_v<DecayT, float>) {
      return ArgType::kFloat;
    } else if constexpr (std::is_same_v<DecayT, double>) {
      return ArgType::kDouble;
    } else if constexpr (std::is_same_v<DecayT, bool>) {
      return ArgType::kBool;
    } else if constexpr (std::is_same_v<DecayT, char>) {
      return ArgType::kChar;
    } else if constexpr (is_dynamic_string) {
      return ArgType::kString;
    } else if constexpr (is_fixed_string) {
      return ArgType::kFixedString;
    } else if constexpr (std::is_pointer_v<DecayT>) {
      return ArgType::kPointer;
    } else {
      static_assert(sizeof(DecayT) == 0, "Unsupported argument type");
    }
  }();

  ArgTypeInfo() = delete;
};

// FixedString<N> -> kFixedString
template <std::size_t N>
inline void write_arg(StringRegistry* registry,
                      char*& pos,
                      const FixedString<N>& value) {
  constexpr StringId id = get_format_id<value>();
  constexpr ArgHeader header(ArgType::kFixedString, sizeof(id));
  registry->register_string<value>();

  std::memcpy(pos, &header, sizeof(header));
  pos += sizeof(header);
  std::memcpy(pos, &id, sizeof(id));
  pos += sizeof(id);
}

// std::string_view -> kString
inline void write_arg(StringRegistry* registry,
                      char*& pos,
                      const std::string_view& view) {
  FEMTOLOG_DCHECK(registry);
  StringId id = hash_string_from_pointer(view.data());
  registry->register_string_arena(id, view);
  constexpr ArgHeader header(ArgType::kString, sizeof(id));
  std::memcpy(pos, &header, sizeof(header));
  pos += sizeof(header);
  std::memcpy(pos, &id, sizeof(id));
  pos += sizeof(id);
}

// char array → kString
template <std::size_t N>
inline void write_arg(StringRegistry* registry,
                      char*& pos,
                      const char (&value)[N]) {
  constexpr std::size_t len = N - 1;
  std::string_view view(value, len);
  write_arg(registry, pos, view);
}

// const char* -> kString
inline void write_arg(StringRegistry* registry,
                      char*& pos,
                      const char*& value) {
  if (value == nullptr) [[unlikely]] {
    write_arg(registry, pos, "nullptr");
    return;
  }
  std::string_view view(value);
  write_arg(registry, pos, view);
}

// const std::string& -> kString
inline void write_arg(StringRegistry* registry,
                      char*& pos,
                      const std::string& value) {
  std::string_view view(value);
  write_arg(registry, pos, view);
}

template <typename T>
inline void write_arg(StringRegistry* registry, char*& pos, const T& value) {
  if constexpr (ArgTypeInfo<T>::is_string_like) {
    if constexpr (ArgTypeInfo<T>::is_fixed_string) {
      static_assert(false, "should not reach this branch");
    } else if constexpr (ArgTypeInfo<T>::is_dynamic_string) {
      if constexpr (ArgTypeInfo<T>::is_char_array) {
        if (value == nullptr) [[unlikely]] {
          write_arg(registry, pos, "nullptr");
          return;
        }
      }
      std::string_view view(value);
      StringId id = hash_string_from_pointer(view.data());

      FEMTOLOG_DCHECK(registry);
      registry->register_string_arena(id, view);
      constexpr ArgHeader header(ArgType::kString, sizeof(id));
      std::memcpy(pos, &header, sizeof(header));
      pos += sizeof(header);
      std::memcpy(pos, &id, sizeof(id));
      pos += sizeof(id);
    }
  } else {
    constexpr ArgHeader header(ArgTypeInfo<T>::type,
                               static_cast<uint16_t>(sizeof(T)));
    std::memcpy(pos, &header, sizeof(header));
    pos += sizeof(header);
    std::memcpy(pos, &value, sizeof(T));
    pos += sizeof(T);
  }
}

template <typename... Args>
constexpr std::size_t calculate_serialized_size() {
  std::size_t total = sizeof(SerializedArgsHeader);

  auto add_arg_size = []<typename T>() constexpr -> std::size_t {
    if constexpr (ArgTypeInfo<T>::is_string_like) {
      return sizeof(ArgHeader) + sizeof(StringId);
    } else {
      return sizeof(ArgHeader) + sizeof(std::decay_t<T>);
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

  // for 0 args -> header only
  [[nodiscard, gnu::hot]] inline static SerializedArgs<64>& serialize(
      StringRegistry*) {
    static SerializedArgs<64> header_only = [] {
      SerializedArgs<64> args;
      constexpr uint32_t header_size = sizeof(SerializedArgsHeader);
      constexpr SerializedArgsHeader header(header_size, 0);

      std::memcpy(args.data(), &header, sizeof(header));
      args.resize(header_size);
      return args;
    }();
    return header_only;
  }

  // for 1 arg
  template <typename T>
  [[nodiscard, gnu::hot]] constexpr SerializedArgs<Capacity>& serialize(
      StringRegistry* registry,
      T&& arg) {
    validate_buffer<T>();

    char* pos = payload_pos();

    write_arg(registry, pos, arg);

    const uint32_t total_size = static_cast<uint32_t>(pos - args_.data());
    const SerializedArgsHeader header(total_size, 1);
    std::memcpy(args_.data(), &header, sizeof(header));

    args_.resize(total_size);
    return args_;
  }

  // for 2 args
  template <typename T1, typename T2>
  [[nodiscard, gnu::hot]] constexpr SerializedArgs<Capacity>&
  serialize(StringRegistry* registry, T1&& arg1, T2&& arg2) {
    validate_buffer<T1, T2>();

    char* pos = payload_pos();

    write_arg(registry, pos, arg1);
    write_arg(registry, pos, arg2);

    const uint32_t total_size = static_cast<uint32_t>(pos - args_.data());
    const SerializedArgsHeader header(total_size, 2);
    std::memcpy(args_.data(), &header, sizeof(header));

    args_.resize(total_size);
    return args_;
  }

  template <typename... Args>
  [[nodiscard, gnu::hot]] constexpr SerializedArgs<Capacity>& serialize(
      StringRegistry* registry,
      Args&&... args) {
    validate_buffer<Args...>();

    char* pos = payload_pos();

    (write_arg(registry, pos, args), ...);

    // auto args_tuple = std::forward_as_tuple(std::forward<Args>(args)...);
    // serialize_arg_at<0>(pos, args_tuple);

    // header
    const uint32_t total_size = static_cast<uint32_t>(pos - args_.data());
    const SerializedArgsHeader header(total_size,
                                      static_cast<uint16_t>(sizeof...(args)));
    std::memcpy(args_.data(), &header, sizeof(header));

    args_.resize(total_size);
    return args_;
  }

 private:
  template <typename... Args>
  FEMTOLOG_FORCE_INLINE static consteval void validate_buffer() {
    constexpr std::size_t min = calculate_serialized_size<Args...>();
    static_assert(Capacity >= min, "Buffer too small for arguments");
  }

  // template <std::size_t I, typename Tuple>
  // inline static void serialize_arg_at(char*& pos, const Tuple& args) {
  //   if constexpr (I < std::tuple_size_v<Tuple>) {
  //     write_arg(pos, std::get<I>(args));
  //     serialize_arg_at<I + 1>(pos, args);
  //   }
  // }

  inline char* payload_pos() {
    return args_.data() + sizeof(SerializedArgsHeader);
  }

  alignas(std::hardware_destructive_interference_size)
      SerializedArgs<Capacity> args_;
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
