// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/core/base/string_util.h"

#include <string>

#include "gtest/gtest.h"

namespace femtolog::core {

TEST(StringUtilTest, ToLowerAsciiChar) {
  EXPECT_EQ(to_lower('A'), 'a');
  EXPECT_EQ(to_lower('Z'), 'z');
  EXPECT_EQ(to_lower('a'), 'a');
  EXPECT_EQ(to_lower('0'), '0');
}

TEST(StringUtilTest, ToUpperAsciiChar) {
  EXPECT_EQ(to_upper('a'), 'A');
  EXPECT_EQ(to_upper('z'), 'Z');
  EXPECT_EQ(to_upper('A'), 'A');
  EXPECT_EQ(to_upper('0'), '0');
}

TEST(StringUtilTest, ToLowerAsciiArray) {
  auto result = to_lower("HeLLo");
  EXPECT_STREQ(result.data(), "hello");
}

TEST(StringUtilTest, ToUpperAsciiArray) {
  auto result = to_upper("HeLLo");
  EXPECT_STREQ(result.data(), "HELLO");
}

TEST(StringUtilTest, EncodeEscape) {
  std::string input = "hello\nworld\t";
  auto encoded = encode_escape(input);
  EXPECT_EQ(encoded, "hello\\nworld\\t");
}

TEST(StringUtilTest, DecodeEscape) {
  std::string input = "hello\\nworld\\t";
  auto decoded = decode_escape(input);
  EXPECT_EQ(decoded, "hello\nworld\t");
}

TEST(StringUtilTest, ToLower) {
  std::string input = "HeLLo";
  EXPECT_EQ(to_lower(input), "hello");
  to_lower(&input);
  EXPECT_EQ(input, "hello");
}

TEST(StringUtilTest, ToUpper) {
  std::string input = "HeLLo";
  EXPECT_EQ(to_upper(input), "HELLO");
  to_upper(&input);
  EXPECT_EQ(input, "HELLO");
}

TEST(StringUtilTest, Utf8CharLength) {
  EXPECT_EQ(utf8_char_length(0x24), 1);  // '$'
  EXPECT_EQ(utf8_char_length(0xC2), 2);  // 2-byte start
  EXPECT_EQ(utf8_char_length(0xE2), 3);  // 3-byte start
  EXPECT_EQ(utf8_char_length(0xF0), 4);  // 4-byte start
}

TEST(StringUtilTest, Utf8Truncate) {
  const std::string input = "あいうえお";  // a + cent + euro
  auto truncated = utf8_truncate(input, 2);
  EXPECT_EQ(truncated, "あい");
}

TEST(StringUtilTest, SplitString) {
  auto result = split_string("a,b,c", ",");
  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result.front(), "a");
  result.pop();
  EXPECT_EQ(result.front(), "b");
  result.pop();
  EXPECT_EQ(result.front(), "c");
  result.pop();
}

TEST(StringUtilTest, RemoveBracket) {
  EXPECT_EQ(remove_bracket("start[test] and end"), "start and end");
  EXPECT_EQ(remove_bracket("for (example)"), "for ");
  EXPECT_EQ(remove_bracket("<tag>"), "");
}

TEST(StringUtilTest, SafeStrlen) {
  const char* s = "test";
  EXPECT_EQ(safe_strlen(s), 4);
  EXPECT_EQ(safe_strlen(nullptr), 0);
}

TEST(StringUtilTest, FormatAddressSafe) {
  char buffer[32];
  format_address_safe(0x1234, buffer, sizeof(buffer));
  EXPECT_STRNE(buffer, "");
}

TEST(StringUtilTest, PaddingAndWriteRaw) {
  char buffer[64] = {};
  char* cursor = buffer;
  const char* end = buffer + sizeof(buffer);
  const char* src = "abc";
  std::size_t written = write_raw(cursor, src, 3);
  cursor += written;
  padding(cursor, end, 3, 8);
  EXPECT_GE(cursor - buffer, 8);
  EXPECT_EQ(std::string(buffer, 3), "abc");
}

TEST(StringUtilTest, StartsWith) {
  EXPECT_TRUE(starts_with("hello", "he"));
  EXPECT_FALSE(starts_with("hello", "lo"));
  EXPECT_TRUE(starts_with("hello", "lo", 3));
}

}  // namespace femtolog::core
