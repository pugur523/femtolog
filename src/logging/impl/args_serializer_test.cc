// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "logging/impl/args_serializer.h"

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

}  // namespace femtolog::logging
