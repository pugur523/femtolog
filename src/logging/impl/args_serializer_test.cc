// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/args_serializer.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

#include "gtest/gtest.h"

namespace femtolog::logging {

namespace {

StringRegistry registry;

TEST(ArgsSerializerTest, SerializeBasicTypes) {
  const int i = 42;
  const std::string s = "hello";
  const float f = 1.234;
  const double d = 1.23456789;
  const char* cstr = "world";

  ArgsSerializer<256> serializer;
  auto& args =
      serializer.serialize<"int={}, str={}, float={}, double={}, ptr={}">(
          &registry, i, s, f, d, cstr);

  EXPECT_EQ(args.size(), sizeof(SerializedArgsHeader) + sizeof(i) +
                             sizeof(StringId) + sizeof(float) + sizeof(double) +
                             sizeof(StringId));

  auto header = reinterpret_cast<const SerializedArgsHeader*>(args.data());
  EXPECT_NE(header->deserialize_and_format_func, nullptr);
  EXPECT_NE(header->format_func, nullptr);

  fmt::memory_buffer buf;
  std::size_t n = header->deserialize_and_format_func(
      &buf, header->format_func, &registry,
      args.data() + sizeof(SerializedArgsHeader));

  const char* formatted_str = buf.data();
  EXPECT_EQ(std::string_view(formatted_str, n),
            "int=42, str=hello, float=1.234, double=1.23456789, ptr=world");
}

TEST(ArgsSerializerTest, OutOfScope) {
  ArgsSerializer<256> serializer;
  SerializedArgs<256>* args_ptr = nullptr;

  {
    const int i = 42;
    const char* cstr = "test";

    auto& args = serializer.serialize<"int={}, cstr={}">(&registry, i, cstr);
    args_ptr = &args;
  }

  EXPECT_EQ(args_ptr->size(),
            sizeof(SerializedArgsHeader) + sizeof(int) + sizeof(StringId));

  auto header = reinterpret_cast<const SerializedArgsHeader*>(args_ptr->data());
  EXPECT_NE(header->deserialize_and_format_func, nullptr);
  EXPECT_NE(header->format_func, nullptr);

  fmt::memory_buffer buf;
  std::size_t n = header->deserialize_and_format_func(
      &buf, header->format_func, &registry,
      args_ptr->data() + sizeof(SerializedArgsHeader));

  const char* formatted_str = buf.data();
  EXPECT_EQ(std::string_view(formatted_str, n), "int=42, cstr=test");
}

}  // namespace

}  // namespace femtolog::logging
