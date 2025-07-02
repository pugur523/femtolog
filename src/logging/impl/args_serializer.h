// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef LOGGING_IMPL_ARGS_SERIALIZER_H_
#define LOGGING_IMPL_ARGS_SERIALIZER_H_

#include <array>
#include <cstdint>
#include <cstring>
#include <new>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

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
  kStringView = 8,
  kCstring = 9,
  kPointer = 10
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

template <std::size_t BufSize = 512>
class alignas(std::hardware_destructive_interference_size) SerializedArgs {
 public:
  SerializedArgs() = default;
  ~SerializedArgs() = default;

  SerializedArgs(const SerializedArgs&) = delete;
  SerializedArgs& operator=(const SerializedArgs&) = delete;

  SerializedArgs(SerializedArgs&& other) noexcept
      : buffer_(std::move(other.buffer_)), data_size_(other.data_size_) {
    other.data_size_ = 0;
  }

  SerializedArgs& operator=(SerializedArgs&& other) noexcept {
    if (this != &other) {
      buffer_ = std::move(other.buffer_);
      data_size_ = other.data_size_;
      other.data_size_ = 0;
    }
    return *this;
  }

  [[nodiscard]] inline const char* data() const { return buffer_.data(); }
  [[nodiscard]] inline std::size_t size() const { return data_size_; }
  [[nodiscard]] inline bool empty() const { return data_size_ == 0; }

  inline char* data() { return buffer_.data(); }
  inline void size(std::size_t size) { data_size_ = size; }
  [[nodiscard]] inline static constexpr std::size_t capacity() {
    return BufSize;
  }

 private:
  alignas(std::hardware_constructive_interference_size)
      std::array<char, BufSize> buffer_;
  std::size_t data_size_ = 0;
};

// detect constexpr string
template <typename T>
constexpr bool is_constexpr_string() {
  if constexpr (std::is_array_v<std::remove_reference_t<T>>) {
    return std::is_same_v<std::remove_extent_t<std::remove_reference_t<T>>,
                          const char>;
  }
  return false;
}

template <typename T>
constexpr std::size_t get_constexpr_string_size() {
  if constexpr (is_constexpr_string<T>()) {
    // exclude null termination
    return std::extent_v<std::remove_reference_t<T>> - 1;
  }
  return 0;
}

template <typename T>
struct ArgTypeInfo {
  using DecayT = std::decay_t<T>;

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
    } else if constexpr (std::is_same_v<DecayT, std::string_view>) {
      return ArgType::kStringView;
    } else if constexpr (std::is_same_v<DecayT, std::string>) {
      return ArgType::kStringView;
    } else if constexpr (std::is_same_v<DecayT, const char*>) {
      return ArgType::kCstring;
    } else if constexpr (is_constexpr_string<T>()) {
      return ArgType::kCstring;
    } else if constexpr (std::is_pointer_v<DecayT>) {
      return ArgType::kPointer;
    } else {
      static_assert(sizeof(DecayT) == 0, "Unsupported argument type");
    }
  }();

  static constexpr bool is_constexpr_string_v = is_constexpr_string<T>();

  static constexpr bool is_dynamic_string =
      (std::is_same_v<DecayT, std::string_view> ||
       std::is_same_v<DecayT, const char*> ||
       std::is_same_v<DecayT, std::string>) &&
      !is_constexpr_string_v;

  static constexpr bool is_string_like =
      is_dynamic_string || is_constexpr_string_v;

  static constexpr std::size_t constexpr_string_size =
      get_constexpr_string_size<T>();

  static constexpr std::size_t fixed_size =
      is_dynamic_string
          ? 0
          : (is_constexpr_string_v ? constexpr_string_size : sizeof(DecayT));

  static constexpr ArgHeader header = ArgHeader{
      type, static_cast<uint16_t>(is_constexpr_string_v ? constexpr_string_size
                                                        : sizeof(DecayT))};
};

template <typename... Args>
static constexpr std::size_t calculate_min_serialized_size() {
  std::size_t total = sizeof(SerializedArgsHeader);

  auto add_arg_size = []<typename T>() constexpr -> std::size_t {
    if constexpr (ArgTypeInfo<T>::is_dynamic_string) {
      return sizeof(ArgHeader);
    } else if constexpr (ArgTypeInfo<T>::is_constexpr_string_v) {
      return sizeof(ArgHeader) + ArgTypeInfo<T>::constexpr_string_size;
    } else {
      return sizeof(ArgHeader) + sizeof(std::decay_t<T>);
    }
  };

  ((total += add_arg_size.template operator()<Args>()), ...);
  return total;
}

// calculate exact serialized size for constexpr strings
template <typename... Args>
static constexpr std::size_t calculate_exact_serialized_size() {
  std::size_t total = sizeof(SerializedArgsHeader);

  auto add_arg_size = []<typename T>() constexpr -> std::size_t {
    if constexpr (ArgTypeInfo<T>::is_constexpr_string_v) {
      return sizeof(ArgHeader) + ArgTypeInfo<T>::constexpr_string_size;
    } else if constexpr (!ArgTypeInfo<T>::is_dynamic_string) {
      return sizeof(ArgHeader) + sizeof(std::decay_t<T>);
    } else {
      // has dynamic string characters; cannot calculate at compile time
      return 0;
    }
  };

  return ((total += add_arg_size.template operator()<Args>()), ...);
}

template <typename... Args>
static constexpr bool all_args_fixed_size() {
  return ((!ArgTypeInfo<Args>::is_dynamic_string) && ...);
}

template <std::size_t Size, typename T>
inline void write_fixed_arg_fast(char*& pos, const T& value, ArgType type) {
  std::memcpy(pos, &type, sizeof(type));
  pos += sizeof(ArgType);

  uint16_t sz = static_cast<uint16_t>(Size);
  std::memcpy(pos, &sz, sizeof(sz));
  pos += sizeof(uint16_t);

  std::memcpy(pos, &value, Size);
  pos += Size;
}

template <typename T>
inline void write_optimized_fixed_arg(char*& pos, const T& value) {
  constexpr ArgType type = ArgTypeInfo<T>::type;
  constexpr std::size_t size = sizeof(T);

  if constexpr (type == ArgType::kInt32 || type == ArgType::kUint32) {
    [[likely]] write_fixed_arg_fast<4>(pos, value, type);
  } else if constexpr (type == ArgType::kInt64 || type == ArgType::kUint64) {
    write_fixed_arg_fast<8>(pos, value, type);
  } else if constexpr (size == 1) {
    write_fixed_arg_fast<1>(pos, value, type);
  } else if constexpr (size == 2) {
    write_fixed_arg_fast<2>(pos, value, type);
  } else {
    write_fixed_arg_fast<size>(pos, value, type);
  }
}

// write arg for constexpr strings
template <std::size_t Size>
[[gnu::always_inline, gnu::hot]]
inline void write_constexpr_string_arg(char*& pos,
                                       const char (&str)[Size],
                                       ArgType type) {
  constexpr uint16_t len = Size - 1;

  std::memcpy(pos, &type, sizeof(type));
  pos += sizeof(ArgType);

  std::memcpy(pos, &len, sizeof(len));
  pos += sizeof(uint16_t);

  if constexpr (len > 0) {
    std::memcpy(pos, str, len);
    pos += len;
  }
}

template <typename T>
[[gnu::always_inline, gnu::hot]]
inline void write_optimized_arg(char*& pos, const T& value) {
  if constexpr (ArgTypeInfo<T>::is_constexpr_string_v) {
    write_constexpr_string_arg(pos, value, ArgTypeInfo<T>::type);
  } else if constexpr (ArgTypeInfo<T>::is_dynamic_string) {
    std::string_view sv;
    if constexpr (std::is_same_v<std::decay_t<T>, std::string_view>) {
      sv = value;
    } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
      sv = std::string_view(value);
    } else {
      sv = std::string_view(value);
    }
    write_string_arg_optimized(pos, sv, ArgTypeInfo<T>::type);
  } else {
    write_optimized_fixed_arg(pos, value);
  }
}

template <std::size_t N>
inline void write_optimized_arg(char*& pos, const char (&value)[N]) {
  write_constexpr_string_arg<N>(pos, value, ArgType::kCstring);
}

[[gnu::always_inline, gnu::hot]]
inline void write_string_arg_optimized(char*& pos,
                                       std::string_view sv,
                                       ArgType type) {
  const uint16_t len = static_cast<uint16_t>(sv.size());

  std::memcpy(pos, &type, sizeof(type));
  pos += sizeof(ArgType);

  std::memcpy(pos, &len, sizeof(len));
  pos += sizeof(uint16_t);

  if (len == 0) [[unlikely]] {
    return;
  } else if (len <= 32) [[likely]] {
    if (len <= 16) {
      switch (len) {
        case 1: *pos = sv[0]; break;
        case 2: std::memcpy(pos, sv.data(), 2); break;
        case 4: std::memcpy(pos, sv.data(), 4); break;
        case 8: std::memcpy(pos, sv.data(), 8); break;
        case 16: std::memcpy(pos, sv.data(), 16); break;
        default: std::memcpy(pos, sv.data(), len); break;
      }
    } else {
      std::memcpy(pos, sv.data(), 16);
      std::memcpy(pos + 16, sv.data() + 16, len - 16);
    }
  } else {
    std::memcpy(pos, sv.data(), len);
  }
  pos += len;
}

template <std::size_t I, typename Tuple>
static constexpr void serialize_arg_at(char*& pos, const Tuple& args) {
  if constexpr (I < std::tuple_size_v<Tuple>) {
    auto&& arg = std::get<I>(args);

    write_optimized_arg(pos, arg);

    serialize_arg_at<I + 1>(pos, args);
  }
}

template <std::size_t BufferSize = 512>
class ArgsSerializer {
 public:
  ArgsSerializer() = default;

  // 0 args
  [[nodiscard, gnu::always_inline, gnu::hot]]
  static constexpr SerializedArgs<BufferSize> serialize() {
    SerializedArgs<BufferSize> result;
    char* buffer = result.data();

    constexpr uint32_t total_size = sizeof(SerializedArgsHeader);
    constexpr SerializedArgsHeader header{total_size, 0};

    std::memcpy(buffer, &header, sizeof(header));
    result.size(total_size);
    return result;
  }

  // 1 arg
  template <typename T>
  [[nodiscard, gnu::always_inline, gnu::hot]]
  static SerializedArgs<BufferSize> serialize(T&& arg) {
    if constexpr (all_args_fixed_size<T>()) {
      constexpr std::size_t exact_size = calculate_exact_serialized_size<T>();
      static_assert(BufferSize >= exact_size, "Buffer too small for argument");
    } else {
      constexpr std::size_t min_size = calculate_min_serialized_size<T>();
      static_assert(BufferSize >= min_size, "Buffer too small for argument");
    }

    SerializedArgs<BufferSize> result;
    char* pos = result.data() + sizeof(SerializedArgsHeader);

    write_optimized_arg(pos, arg);

    // header
    char* buffer = result.data();
    const uint32_t total_size = static_cast<uint32_t>(pos - buffer);
    const SerializedArgsHeader header{total_size, 1};
    std::memcpy(buffer, &header, sizeof(header));

    result.size(total_size);
    return result;
  }

  // 2 args
  template <typename T1, typename T2>
  [[nodiscard, gnu::always_inline, gnu::hot]]
  static SerializedArgs<BufferSize> serialize(T1&& arg1, T2&& arg2) {
    if constexpr (all_args_fixed_size<T1, T2>()) {
      constexpr std::size_t exact_size =
          calculate_exact_serialized_size<T1, T2>();
      static_assert(BufferSize >= exact_size, "Buffer too small for arguments");
    } else {
      constexpr std::size_t min_size = calculate_min_serialized_size<T1, T2>();
      static_assert(BufferSize >= min_size, "Buffer too small for arguments");
    }

    SerializedArgs<BufferSize> result;
    char* pos = result.data() + sizeof(SerializedArgsHeader);

    write_optimized_arg(pos, arg1);
    write_optimized_arg(pos, arg2);

    // header
    char* buffer = result.data();
    const uint32_t total_size = static_cast<uint32_t>(pos - buffer);
    const SerializedArgsHeader header{total_size, 2};
    std::memcpy(buffer, &header, sizeof(header));

    result.size(total_size);
    return result;
  }

  // variable args
  template <typename... Args>
  [[nodiscard, gnu::flatten]]
  static SerializedArgs<BufferSize> serialize(Args&&... args)
    requires(sizeof...(args) > 2)
  {  // NOLINT
    if constexpr (all_args_fixed_size<Args...>()) {
      constexpr std::size_t exact_size =
          calculate_exact_serialized_size<Args...>();
      static_assert(BufferSize >= exact_size, "Buffer too small for arguments");
    } else {
      constexpr std::size_t min_size = calculate_min_serialized_size<Args...>();
      static_assert(BufferSize >= min_size, "Buffer too small for arguments");
    }

    SerializedArgs<BufferSize> result;
    char* pos = result.data() + sizeof(SerializedArgsHeader);

    // make args tuple and process at compile time
    auto args_tuple = std::forward_as_tuple(std::forward<Args>(args)...);
    serialize_arg_at<0>(pos, args_tuple);

    // header
    char* buffer = result.data();
    const uint32_t total_size = static_cast<uint32_t>(pos - buffer);
    const SerializedArgsHeader header{total_size,
                                      static_cast<uint16_t>(sizeof...(args))};
    std::memcpy(buffer, &header, sizeof(header));

    result.size(total_size);
    return result;
  }
};

using DefaultSerializer = ArgsSerializer<512>;
using SmallSerializer = ArgsSerializer<256>;
using LargeSerializer = ArgsSerializer<4096>;

using DefaultSerializedArgs = SerializedArgs<512>;
using SmallSerializedArgs = SerializedArgs<256>;
using LargeSerializedArgs = SerializedArgs<4096>;

}  // namespace femtolog::logging

#endif  // LOGGING_IMPL_ARGS_SERIALIZER_H_
