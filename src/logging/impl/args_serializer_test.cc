// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/args_serializer.h"

#include <cstdint>
#include <cstring>
#include <string_view>

#include "gtest/gtest.h"

namespace femtolog::logging {

TEST(ArgsSerializerTest, SerializeEmpty) {
  DefaultSerializer serializer;
  auto result = serializer.serialize();
  ASSERT_EQ(result.size(), sizeof(SerializedArgsHeader));

  auto* header = reinterpret_cast<const SerializedArgsHeader*>(result.data());
  EXPECT_EQ(header->arg_count, 0);
  EXPECT_EQ(header->total_size, sizeof(SerializedArgsHeader));
}

TEST(ArgsSerializerTest, SerializeInts) {
  DefaultSerializer serializer;
  auto result = serializer.serialize(1, 42u, static_cast<int64_t>(-5));

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 3);
  EXPECT_EQ(header->total_size, result.size());
}

TEST(ArgsSerializerTest, SerializeStrings) {
  DefaultSerializer serializer;
  const char* cstr = "hello";
  std::string_view sv = "world";
  auto result = serializer.serialize(cstr, sv);

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 2);
  EXPECT_EQ(header->total_size, result.size());
}

TEST(ArgsSerializerTest, SerializeInt32CheckLayout) {
  DefaultSerializer serializer;
  int32_t value = 12345;
  auto result = serializer.serialize(value);

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 1);
  EXPECT_EQ(header->total_size, result.size());

  const char* arg_ptr = ptr + sizeof(SerializedArgsHeader);
  auto* arg_header = reinterpret_cast<const ArgHeader*>(arg_ptr);
  EXPECT_EQ(arg_header->type, ArgType::kInt32);
  EXPECT_EQ(arg_header->size, sizeof(int32_t));

  const int32_t* val_ptr =
      reinterpret_cast<const int32_t*>(arg_ptr + sizeof(ArgHeader));
  EXPECT_EQ(*val_ptr, value);
}

TEST(ArgsSerializerTest, SerializeCStringCheckLayout) {
  DefaultSerializer serializer;
  const char* cstr = "hi";
  auto result = serializer.serialize(cstr);

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 1);
  EXPECT_EQ(header->total_size, result.size());

  const char* arg_ptr = ptr + sizeof(SerializedArgsHeader);
  auto* arg_header = reinterpret_cast<const ArgHeader*>(arg_ptr);
  EXPECT_EQ(arg_header->type, ArgType::kCstring);
  EXPECT_EQ(arg_header->size, 2);

  const char* str_data = arg_ptr + sizeof(ArgHeader);
  EXPECT_EQ(std::string_view(str_data, arg_header->size), "hi");
}

TEST(ArgsSerializerTest, SerializeNullptrCString) {
  DefaultSerializer serializer;
  const char* cstr = nullptr;
  auto result = serializer.serialize(cstr);

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 1);
  EXPECT_EQ(header->total_size, result.size());

  const char* arg_ptr = ptr + sizeof(SerializedArgsHeader);
  auto* arg_header = reinterpret_cast<const ArgHeader*>(arg_ptr);
  EXPECT_EQ(arg_header->type, ArgType::kCstring);
  EXPECT_EQ(arg_header->size, 7);  // "nullptr"

  const char* str_data = arg_ptr + sizeof(ArgHeader);
  EXPECT_EQ(std::string_view(str_data, arg_header->size), "nullptr");
}

TEST(ArgsSerializerTest, SerializeStringViewCheckLayout) {
  DefaultSerializer serializer;
  std::string_view sv = "abc";
  auto result = serializer.serialize(sv);

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 1);
  EXPECT_EQ(header->total_size, result.size());

  const char* arg_ptr = ptr + sizeof(SerializedArgsHeader);
  auto* arg_header = reinterpret_cast<const ArgHeader*>(arg_ptr);
  EXPECT_EQ(arg_header->type, ArgType::kStringView);
  EXPECT_EQ(arg_header->size, 3);

  const char* str_data = arg_ptr + sizeof(ArgHeader);
  EXPECT_EQ(std::string_view(str_data, arg_header->size), "abc");
}

TEST(ArgsSerializerTest, SerializeCharPointerLifetimeIndependent) {
  DefaultSerializedArgs result;

  {
    char temp[] = "scoped";
    result = DefaultSerializer::serialize(temp);

    ASSERT_EQ(result.size() > sizeof(SerializedArgsHeader), true);
  }

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 1);
  EXPECT_EQ(header->total_size, result.size());

  const char* arg_ptr = ptr + sizeof(SerializedArgsHeader);
  auto* arg_header = reinterpret_cast<const ArgHeader*>(arg_ptr);
  EXPECT_EQ(arg_header->type, ArgType::kCstring);
  EXPECT_EQ(arg_header->size, 6);

  const char* str_data = arg_ptr + sizeof(ArgHeader);
  EXPECT_EQ(std::string_view(str_data, arg_header->size), "scoped");
}

}  // namespace femtolog::logging
