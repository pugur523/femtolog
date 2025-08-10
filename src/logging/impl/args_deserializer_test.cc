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

TEST(ArgsDeserializerTest, DeserializeAndFormatWorks) {
  StringRegistry registry;

  int i = 42;
  std::string s = "example";
  double d = 3.14;

  ArgsSerializer<256> serializer;
  auto& args = serializer.serialize<"i={}, s={}, d={}">(i, s, d);

  const SerializedArgsHeader* header =
      reinterpret_cast<const SerializedArgsHeader*>(args.data());

  fmt::memory_buffer buffer;
  std::size_t size = header->deserialize_and_format_func(
      &buffer, header->format_func,
      reinterpret_cast<const char*>(args.data() + sizeof(*header)));

  std::string_view formatted(buffer.data(), size);
  EXPECT_EQ(formatted, "i=42, s=example, d=3.14");
}

}  // namespace

}  // namespace femtolog::logging
