// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <random>
#include <string>
#include <vector>

#include "benchmark/benchmark.h"
#include "femtolog/core/base/string_util.h"

namespace core {

namespace {

const char* kShortString = "hello world";
const char* kMediumString = "The quick brown fox jumps over the lazy dog.";
const std::string kLongString = [] {  // NOLINT
  std::string s;
  s.reserve(1024 * 10);  // 10 KiB
  for (int i = 0; i < 1000; ++i) {
    s += "A very long string for benchmarking purposes. ";
  }
  return s;
}();

constexpr const char* kStringWithEscape = "hello world\\n\\t\\r\\\"test\\\'";
constexpr const char* kUtf8String = "こんにちは世界🌍";  // 15 chars, 20 bytes
constexpr const char* kSplitString = "one,two,three,four,five";
constexpr const char* kBracketString = "(abc)[def]{ghi}";
constexpr const char* kFormatString = "Value: {}, Text: {}";
constexpr const char* kPrefixString = "prefix_test_string";

std::mt19937 g_rng(std::random_device{}());  // NOLINT

std::string generate_random_string(std::size_t length) {
  std::string s(length, '\0');
  static const char charset[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::uniform_int_distribution<> distrib(0, sizeof(charset) - 2);
  for (std::size_t i = 0; i < length; ++i) {
    s[i] = charset[distrib(g_rng)];
  }
  return s;
}

void string_util_encode_escape_short(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(
        encode_escape(std::string_view(kStringWithEscape)));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(kStringWithEscape));
}
BENCHMARK(string_util_encode_escape_short);

void string_util_encode_escape_long(benchmark::State& state) {
  std::string long_escape_string = kLongString;
  for (int i = 0; i < 1000; ++i) {
    long_escape_string += "\\n";
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(encode_escape(long_escape_string));
  }
  state.SetBytesProcessed(state.iterations() * long_escape_string.size());
}
BENCHMARK(string_util_encode_escape_long);

void string_util_decode_escape_short(benchmark::State& state) {
  std::string encoded = encode_escape(std::string_view(kStringWithEscape));
  for (auto _ : state) {
    benchmark::DoNotOptimize(decode_escape(encoded));
  }
  state.SetBytesProcessed(state.iterations() * encoded.size());
}
BENCHMARK(string_util_decode_escape_short);

void string_util_decode_escape_long(benchmark::State& state) {
  std::string long_escape_string = kLongString;
  for (int i = 0; i < 1000; ++i) {
    long_escape_string += "\\n";
  }
  std::string encoded = encode_escape(long_escape_string);
  for (auto _ : state) {
    benchmark::DoNotOptimize(decode_escape(encoded));
  }
  state.SetBytesProcessed(state.iterations() * encoded.size());
}
BENCHMARK(string_util_decode_escape_long);

void string_util_to_lower_char_ptr(benchmark::State& state) {
  char* buf = new char[kLongString.length() + 1];
  write_raw(buf, kLongString.c_str(), kLongString.length());
  for (auto _ : state) {
    to_lower(buf);
    benchmark::DoNotOptimize(buf);
    write_raw(buf, kLongString.c_str(), kLongString.length());
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
  delete[] buf;
}
BENCHMARK(string_util_to_lower_char_ptr);

void string_util_to_lower_char_ptr_w_len(benchmark::State& state) {
  char* buf = new char[kLongString.length() + 1];
  write_raw(buf, kLongString.c_str(), kLongString.length());
  for (auto _ : state) {
    to_lower(buf, kLongString.length());
    benchmark::DoNotOptimize(buf);
    write_raw(buf, kLongString.c_str(), kLongString.length());
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
  delete[] buf;
}
BENCHMARK(string_util_to_lower_char_ptr_w_len);

void string_util_to_lower_string_ptr(benchmark::State& state) {
  std::string s = kLongString;
  for (auto _ : state) {
    to_lower(&s);
    benchmark::DoNotOptimize(s);
    s = kLongString;
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
}
BENCHMARK(string_util_to_lower_string_ptr);

void string_util_to_lower_const_string_ref(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_lower(kLongString));
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
}
BENCHMARK(string_util_to_lower_const_string_ref);

void string_util_to_upper_char_ptr(benchmark::State& state) {
  char* buf = new char[kLongString.length() + 1];
  write_raw(buf, kLongString.c_str(), kLongString.length());
  for (auto _ : state) {
    to_upper(buf);
    benchmark::DoNotOptimize(buf);
    write_raw(buf, kLongString.c_str(), kLongString.length());
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
  delete[] buf;
}
BENCHMARK(string_util_to_upper_char_ptr);

void string_util_to_upper_char_ptr_w_len(benchmark::State& state) {
  char* buf = new char[kLongString.length() + 1];
  write_raw(buf, kLongString.c_str(), kLongString.length());
  for (auto _ : state) {
    to_upper(buf, kLongString.length());
    benchmark::DoNotOptimize(buf);
    write_raw(buf, kLongString.c_str(), kLongString.length());
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
  delete[] buf;
}
BENCHMARK(string_util_to_upper_char_ptr_w_len);

void string_util_to_upper_string_ptr(benchmark::State& state) {
  std::string s = kLongString;
  for (auto _ : state) {
    to_upper(&s);
    benchmark::DoNotOptimize(s);
    s = kLongString;
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
}
BENCHMARK(string_util_to_upper_string_ptr);

void string_util_to_upper_const_string_ref(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(to_upper(kLongString));
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
}
BENCHMARK(string_util_to_upper_const_string_ref);

void string_util_utf8_char_length(benchmark::State& state) {
  const unsigned char lead_byte_ascii = 'A';
  const unsigned char lead_byte_2byte = 0xC2;  // Example for '¢'
  const unsigned char lead_byte_3byte = 0xE2;  // Example for '€'
  const unsigned char lead_byte_4byte = 0xF0;  // Example for '𐐷'

  for (auto _ : state) {
    benchmark::DoNotOptimize(utf8_char_length(lead_byte_ascii));
    benchmark::DoNotOptimize(utf8_char_length(lead_byte_2byte));
    benchmark::DoNotOptimize(utf8_char_length(lead_byte_3byte));
    benchmark::DoNotOptimize(utf8_char_length(lead_byte_4byte));
  }
  state.SetBytesProcessed(state.iterations() * (1 + 2 + 3 + 4));
}
BENCHMARK(string_util_utf8_char_length);

void string_util_utf8_truncate_no_truncate(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(utf8_truncate(kUtf8String, 20));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(kUtf8String));
}
BENCHMARK(string_util_utf8_truncate_no_truncate);

void string_util_utf8_truncate_truncate_mid(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(utf8_truncate(kUtf8String, 5));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(kUtf8String));
}
BENCHMARK(string_util_utf8_truncate_truncate_mid);

void string_util_utf8_truncate_long_string(benchmark::State& state) {
  std::string long_utf8_string = kLongString;
  long_utf8_string += kUtf8String;
  long_utf8_string += kUtf8String;

  for (auto _ : state) {
    benchmark::DoNotOptimize(
        utf8_truncate(long_utf8_string, long_utf8_string.length() / 2));
  }
  state.SetBytesProcessed(state.iterations() * long_utf8_string.length() / 2);
}
BENCHMARK(string_util_utf8_truncate_long_string);

void string_util_split_string_comma(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(split_string(kSplitString, ","));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(kSplitString));
}
BENCHMARK(string_util_split_string_comma);

void string_util_split_string_long(benchmark::State& state) {
  std::string long_split_string;
  for (int i = 0; i < 1000; ++i) {
    long_split_string += "item" + std::to_string(i) + ",";
  }
  long_split_string.pop_back();

  for (auto _ : state) {
    benchmark::DoNotOptimize(split_string(long_split_string, ","));
  }
  state.SetBytesProcessed(state.iterations() * long_split_string.size());
}
BENCHMARK(string_util_split_string_long);

void string_util_remove_bracket_simple(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(remove_bracket(kBracketString));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(kBracketString));
}
BENCHMARK(string_util_remove_bracket_simple);

void string_util_remove_bracket_no_bracket(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(remove_bracket(kShortString));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(kShortString));
}
BENCHMARK(string_util_remove_bracket_no_bracket);

void string_util_remove_bracket_no_bracket_long(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(remove_bracket(kLongString));
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
}
BENCHMARK(string_util_remove_bracket_no_bracket_long);

void string_util_remove_bracket_long(benchmark::State& state) {
  std::string long_bracket_string = kLongString;
  long_bracket_string += "(extra){brackets}[here]";
  for (auto _ : state) {
    benchmark::DoNotOptimize(remove_bracket(long_bracket_string));
  }
  state.SetBytesProcessed(state.iterations() * long_bracket_string.size());
}
BENCHMARK(string_util_remove_bracket_long);

void string_util_safe_strlen(benchmark::State& state) {
  const char* c_str = kLongString.c_str();
  for (auto _ : state) {
    benchmark::DoNotOptimize(safe_strlen(c_str));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(c_str));
}
BENCHMARK(string_util_safe_strlen);

void string_util_safe_strlen_short(benchmark::State& state) {
  const char* c_str = kShortString;
  for (auto _ : state) {
    benchmark::DoNotOptimize(safe_strlen(c_str));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(c_str));
}
BENCHMARK(string_util_safe_strlen_short);

void string_util_padding(benchmark::State& state) {
  const std::size_t buffer_size = 1024;
  std::vector<char> buffer(buffer_size);
  char* cursor;
  const char* end = buffer.data() + buffer_size;

  for (auto _ : state) {
    cursor = buffer.data();
    std::size_t current_len = 50;
    std::size_t align_len = 8;
    padding(cursor, end, current_len, align_len);
    benchmark::DoNotOptimize(cursor);
  }
  state.SetBytesProcessed(state.iterations() * buffer.size());
}
BENCHMARK(string_util_padding);

void string_util_write_raw(benchmark::State& state) {
  const std::size_t buffer_size = 1024;
  std::vector<char> buffer(buffer_size);
  const char* source_data = kMediumString;
  std::size_t source_len = safe_strlen(kMediumString);

  for (auto _ : state) {
    char* cursor = buffer.data();
    benchmark::DoNotOptimize(write_raw(cursor, source_data, source_len));
  }
  state.SetBytesProcessed(state.iterations() * buffer.size());
}
BENCHMARK(string_util_write_raw);

void string_util_write_raw_long(benchmark::State& state) {
  const std::size_t buffer_size = kLongString.length() + 100;
  std::vector<char> buffer(buffer_size);
  const char* source_data = kLongString.c_str();
  std::size_t source_len = kLongString.length();

  for (auto _ : state) {
    char* cursor = buffer.data();
    benchmark::DoNotOptimize(write_raw(cursor, source_data, source_len));
  }
  state.SetBytesProcessed(state.iterations() * buffer.size());
}
BENCHMARK(string_util_write_raw_long);

void string_util_write_format_int_string(benchmark::State& state) {
  const std::size_t buffer_size = 256;
  std::vector<char> buffer(buffer_size);

  for (auto _ : state) {
    char* cursor = buffer.data();
    const char* end = buffer.data() + buffer_size;
    write_format(cursor, end, kFormatString, 123, "benchmark");
  }
  state.SetBytesProcessed(state.iterations() * buffer_size);
}
BENCHMARK(string_util_write_format_int_string);

void string_util_write_format_long_output(benchmark::State& state) {
  const std::size_t buffer_size = 1024;
  std::vector<char> buffer(buffer_size);
  std::string long_text_arg = generate_random_string(500);

  for (auto _ : state) {
    char* cursor = buffer.data();
    const char* end = buffer.data() + buffer_size;
    write_format(cursor, end, "Long text: {}", long_text_arg.c_str());
  }
  state.SetBytesProcessed(state.iterations() * buffer_size);
}
BENCHMARK(string_util_write_format_long_output);

void string_util_starts_with_match(benchmark::State& state) {
  const std::string prefix = "prefix";
  for (auto _ : state) {
    benchmark::DoNotOptimize(starts_with(kPrefixString, prefix));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(kPrefixString));
}
BENCHMARK(string_util_starts_with_match);

void string_util_starts_with_no_match(benchmark::State& state) {
  const std::string prefix = "nomatch";
  for (auto _ : state) {
    benchmark::DoNotOptimize(starts_with(kPrefixString, prefix));
  }
  state.SetBytesProcessed(state.iterations() * safe_strlen(kPrefixString));
}
BENCHMARK(string_util_starts_with_no_match);

void string_util_starts_with_long_string_match(benchmark::State& state) {
  std::string prefix = kLongString.substr(0, 100);
  for (auto _ : state) {
    benchmark::DoNotOptimize(starts_with(kLongString, prefix));
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
}
BENCHMARK(string_util_starts_with_long_string_match);

void string_util_starts_with_long_string_no_match(benchmark::State& state) {
  std::string prefix = kLongString.substr(0, 99) + "X";
  for (auto _ : state) {
    benchmark::DoNotOptimize(starts_with(kLongString, prefix));
  }
  state.SetBytesProcessed(state.iterations() * kLongString.size());
}
BENCHMARK(string_util_starts_with_long_string_no_match);

}  // namespace

}  // namespace core
