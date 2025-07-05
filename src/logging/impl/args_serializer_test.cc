// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/args_serializer.h"

#include <cstdint>
#include <cstring>
#include <string_view>

#include "gtest/gtest.h"

namespace femtolog::logging {

namespace {

StringRegistry registry;

TEST(ArgsSerializerTest, SerializeEmpty) {
  ArgsSerializer serializer;
  auto result = serializer.serialize(&registry);
  ASSERT_EQ(result.size(), sizeof(SerializedArgsHeader));

  auto* header = reinterpret_cast<const SerializedArgsHeader*>(result.data());
  EXPECT_EQ(header->arg_count, 0);
  EXPECT_EQ(header->total_size, sizeof(SerializedArgsHeader));
}

TEST(ArgsSerializerTest, SerializeInts) {
  ArgsSerializer serializer;
  auto result =
      serializer.serialize(&registry, 1, 42u, static_cast<int64_t>(-5));

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 3);
  EXPECT_EQ(header->total_size, result.size());
}

TEST(ArgsSerializerTest, SerializeStrings) {
  ArgsSerializer serializer;
  const char* cstr = "hello";
  std::string_view sv = "world";
  auto result = serializer.serialize(&registry, cstr, sv);

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 2);
  EXPECT_EQ(header->total_size, result.size());
}

TEST(ArgsSerializerTest, SerializeInt32CheckLayout) {
  ArgsSerializer serializer;
  int32_t value = 12345;
  auto result = serializer.serialize(&registry, value);

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
  ArgsSerializer serializer;
  const char* cstr = "hi";
  auto result = serializer.serialize(&registry, cstr);

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 1);
  EXPECT_EQ(header->total_size, result.size());

  const char* arg_ptr = ptr + sizeof(SerializedArgsHeader);
  auto* arg_header = reinterpret_cast<const ArgHeader*>(arg_ptr);
  EXPECT_EQ(arg_header->type, ArgType::kString);
  EXPECT_EQ(arg_header->size, sizeof(StringId));

  const char* data = arg_ptr + sizeof(ArgHeader);
  StringId id;
  std::memcpy(&id, data, sizeof(StringId));
  EXPECT_EQ(registry.get_string(id), "hi");
}

TEST(ArgsSerializerTest, SerializeNullptrCString) {
  ArgsSerializer serializer;
  const char* cstr = nullptr;
  auto result = serializer.serialize(&registry, cstr);

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 1);
  EXPECT_EQ(header->total_size, result.size());

  const char* arg_ptr = ptr + sizeof(SerializedArgsHeader);
  auto* arg_header = reinterpret_cast<const ArgHeader*>(arg_ptr);
  EXPECT_EQ(arg_header->type, ArgType::kString);
  EXPECT_EQ(arg_header->size, sizeof(StringId));

  const char* data = arg_ptr + sizeof(ArgHeader);
  StringId id;
  std::memcpy(&id, data, sizeof(StringId));
  EXPECT_EQ(registry.get_string(id), "nullptr");
}

TEST(ArgsSerializerTest, SerializeStringViewCheckLayout) {
  ArgsSerializer serializer;
  std::string_view sv = "abc";
  auto result = serializer.serialize(&registry, sv);

  const char* ptr = result.data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 1);
  EXPECT_EQ(header->total_size, result.size());

  const char* arg_ptr = ptr + sizeof(SerializedArgsHeader);
  auto* arg_header = reinterpret_cast<const ArgHeader*>(arg_ptr);
  EXPECT_EQ(arg_header->type, ArgType::kString);
  EXPECT_EQ(arg_header->size, sizeof(StringId));

  const char* data = arg_ptr + sizeof(ArgHeader);
  StringId id;
  std::memcpy(&id, data, sizeof(StringId));
  EXPECT_EQ(registry.get_string(id), "abc");
}

TEST(ArgsSerializerTest, SerializeCharPointerLifetimeIndependent) {
  ArgsSerializer<512> serializer;
  SerializedArgs<512>* result = nullptr;
  {
    char temp[] = "scoped";
    auto& args = serializer.serialize(&registry, temp);
    result = &args;

    ASSERT_EQ(result->size() > sizeof(SerializedArgsHeader), true);
  }

  const char* ptr = result->data();
  auto* header = reinterpret_cast<const SerializedArgsHeader*>(ptr);
  EXPECT_EQ(header->arg_count, 1);
  EXPECT_EQ(header->total_size, result->size());

  const char* arg_ptr = ptr + sizeof(SerializedArgsHeader);
  auto* arg_header = reinterpret_cast<const ArgHeader*>(arg_ptr);
  EXPECT_EQ(arg_header->type, ArgType::kString);
  EXPECT_EQ(arg_header->size, sizeof(StringId));

  const char* data = arg_ptr + sizeof(ArgHeader);
  StringId id;
  std::memcpy(&id, data, sizeof(StringId));
  EXPECT_EQ(registry.get_string(id), "scoped");
}

}  // namespace

}  // namespace femtolog::logging
