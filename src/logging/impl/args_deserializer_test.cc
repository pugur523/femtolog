// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/args_deserializer.h"

#include <string>

#include "femtolog/logging/impl/args_serializer.h"
#include "fmt/format.h"
#include "gtest/gtest.h"

namespace femtolog::logging {

namespace {

StringRegistry registry;

TEST(ArgsDeserializerTest, BasicTypes) {
  ArgsSerializer serializer;
  int32_t a = -42;
  uint64_t b = 1234567890123ULL;
  double c = 3.14159;
  char d = 'x';
  bool e = true;

  auto& data = serializer.serialize(&registry, a, b, c, d, e);
  ArgsDeserializer deserializer(data, &registry);
  auto store = deserializer.deserialize();

  std::string result = fmt::vformat("a={}, b={}, c={}, d={}, e={}", store);
  EXPECT_EQ(result, "a=-42, b=1234567890123, c=3.14159, d=x, e=true");
}

TEST(ArgsDeserializerTest, StringTypes) {
  ArgsSerializer serializer;
  std::string_view sv = "hello";
  const char* cstr = "world";

  auto& data = serializer.serialize(&registry, sv, cstr);
  ArgsDeserializer deserializer(data, &registry);
  auto store = deserializer.deserialize();

  std::string result = fmt::vformat("{} {}", store);
  EXPECT_EQ(result, "hello world");
}

TEST(ArgsDeserializerTest, NullptrCstr) {
  ArgsSerializer serializer;
  const char* null_cstr = nullptr;

  auto& data = serializer.serialize(&registry, null_cstr);
  ArgsDeserializer deserializer(data, &registry);
  auto store = deserializer.deserialize();

  std::string result = fmt::vformat("{}", store);
  EXPECT_EQ(result, "nullptr");
}

TEST(ArgsDeserializerTest, FixedString) {
  ArgsSerializer serializer;
  auto& data = serializer.serialize(&registry, "fixed_string");
  ArgsDeserializer deserializer(data, &registry);
  auto store = deserializer.deserialize();

  std::string result = fmt::vformat("{}", store);
  EXPECT_EQ(result, "fixed_string");
}

}  // namespace

}  // namespace femtolog::logging
