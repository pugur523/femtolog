// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#ifndef INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_DESERIALIZER_H_
#define INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_DESERIALIZER_H_

#include "femtolog/logging/impl/args_serializer.h"
#include "fmt/args.h"

namespace femtolog::logging {

class ArgsDeserializer {
 public:
  explicit ArgsDeserializer(const SerializedArgs<>& serialized_args)
      : data_(serialized_args.data()),
        header_(reinterpret_cast<const SerializedArgsHeader*>(
            serialized_args.data())) {
    pos_ = sizeof(SerializedArgsHeader);
  }

  template <std::size_t BufferSize>
  explicit ArgsDeserializer(const SerializedArgs<BufferSize>& serialized_args)
      : data_(serialized_args.data()),
        header_(reinterpret_cast<const SerializedArgsHeader*>(
            serialized_args.data())) {
    pos_ = sizeof(SerializedArgsHeader);
  }

  explicit ArgsDeserializer(char* src)
      : data_(src),
        header_(reinterpret_cast<const SerializedArgsHeader*>(src)) {
    pos_ = sizeof(SerializedArgsHeader);
  }

  fmt::dynamic_format_arg_store<fmt::format_context> deserialize() {
    fmt::dynamic_format_arg_store<fmt::format_context> store;
    pos_ = sizeof(SerializedArgsHeader);

    for (uint16_t i = 0; i < header_->arg_count; ++i) {
      const ArgHeader* arg_header =
          reinterpret_cast<const ArgHeader*>(data_ + pos_);
      pos_ += sizeof(ArgHeader);

      switch (arg_header->type) {
        case ArgType::kInt32:
          int32_t i32_value;
          std::memcpy(&i32_value, data_ + pos_, sizeof(i32_value));
          store.push_back(i32_value);
          break;
        case ArgType::kInt64:
          int64_t i64_value;
          std::memcpy(&i64_value, data_ + pos_, sizeof(i64_value));
          store.push_back(i64_value);
          break;
        case ArgType::kUint32:
          uint32_t u32_value;
          std::memcpy(&u32_value, data_ + pos_, sizeof(u32_value));
          store.push_back(u32_value);
          break;
        case ArgType::kUint64:
          uint64_t u64_value;
          std::memcpy(&u64_value, data_ + pos_, sizeof(u64_value));
          store.push_back(u64_value);
          break;
        case ArgType::kFloat:
          float f32_value;
          std::memcpy(&f32_value, data_ + pos_, sizeof(f32_value));
          store.push_back(f32_value);
          break;
        case ArgType::kDouble:
          double f64_value;
          std::memcpy(&f64_value, data_ + pos_, sizeof(f64_value));
          store.push_back(f64_value);
          break;
        case ArgType::kBool:
          bool bool_value;
          std::memcpy(&bool_value, data_ + pos_, sizeof(bool_value));
          store.push_back(bool_value);
          break;
        case ArgType::kChar:
          char char_value;
          std::memcpy(&char_value, data_ + pos_, sizeof(char_value));
          store.push_back(char_value);
          break;
        case ArgType::kStringView:
        case ArgType::kCstring: {
          const char* str_data = reinterpret_cast<const char*>(data_ + pos_);
          std::string_view str_view(str_data, arg_header->size);
          store.push_back(str_view);
          break;
        }
        case ArgType::kPointer:
          const void* ptr_value;
          std::memcpy(&ptr_value, data_ + pos_, sizeof(ptr_value));
          store.push_back(ptr_value);
          break;
      }
      pos_ += arg_header->size;
    }

    return store;
  }

  inline uint16_t arg_count() const { return header_->arg_count; }
  inline uint32_t total_size() const { return header_->total_size; }

 private:
  const char* data_;
  std::size_t pos_ = 0;
  const SerializedArgsHeader* header_;
};

}  // namespace femtolog::logging

#endif  // INCLUDE_FEMTOLOG_LOGGING_IMPL_ARGS_DESERIALIZER_H_
