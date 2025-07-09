// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/core/base/string_util.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cstring>
#include <cuchar>
#include <queue>
#include <string>
#include <vector>

namespace femtolog::core {

namespace {

constexpr std::array<int8_t, 256> make_bracket_table() {
  std::array<int8_t, 256> map = {};
  map['('] = 1;
  map[')'] = -1;
  map['{'] = 2;
  map['}'] = -2;
  map['['] = 3;
  map[']'] = -3;
  map['<'] = 4;
  map['>'] = -4;
  return map;
}

void encode_escape_impl(std::string_view input, std::string* out) {
  const std::size_t max_size = input.size() * 2;  // worst case
  out->clear();
  out->reserve(max_size);

  out->resize(max_size);
  char* write_ptr = out->data();
  char* begin = write_ptr;

  for (char c : input) {
    switch (c) {
      case '\n':
        *write_ptr++ = '\\';
        *write_ptr++ = 'n';
        break;
      case '\r':
        *write_ptr++ = '\\';
        *write_ptr++ = 'r';
        break;
      case '\t':
        *write_ptr++ = '\\';
        *write_ptr++ = 't';
        break;
      case '\\':
        *write_ptr++ = '\\';
        *write_ptr++ = '\\';
        break;
      default: *write_ptr++ = c; break;
    }
  }

  out->resize(static_cast<std::size_t>(write_ptr - begin));
}

void decode_escape_impl(std::string_view input, std::string* out) {
  const std::size_t max_size = input.size();  // worst case
  out->clear();
  out->reserve(max_size);

  out->resize(max_size);
  char* write_ptr = out->data();
  char* begin = write_ptr;

  for (std::size_t i = 0; i < input.size(); i++) {
    if (input[i] == '\\') {
      if (i < input.size() - 1) {
        switch (input[i + 1]) {
          case 'n':
            *write_ptr++ = '\n';
            i++;
            break;
          case 'r':
            *write_ptr++ = '\r';
            i++;
            break;
          case 't':
            *write_ptr++ = '\t';
            i++;
            break;
          case '\\':
            *write_ptr++ = '\\';
            i++;
            break;
          default: *write_ptr++ = input[i]; break;
        }
      } else {
        *write_ptr++ = input[i];
      }
    } else {
      *write_ptr++ = input[i];
    }
  }

  out->resize(static_cast<std::size_t>(write_ptr - begin));
}

}  // namespace

std::string encode_escape(std::string_view input) {
  std::string encoded;
  encode_escape_impl(input, &encoded);
  return encoded;
}

std::string encode_escape(const char* s, std::size_t len) {
  return encode_escape(std::string_view(s, len));
}

std::string decode_escape(std::string_view input) {
  std::string decoded;
  decode_escape_impl(input, &decoded);
  return decoded;
}

std::string decode_escape(const char* s, std::size_t len) {
  return decode_escape(std::string_view(s, len));
}

void to_lower(char* input, std::size_t len) {
  if (!input) {
    return;
  }
  for (std::size_t i = 0; i < len; i++) {
    input[i] = to_lower(input[i]);
  }
}

void to_lower(char* input) {
  if (!input) {
    return;
  }
  for (char* p = input; *p != '\0'; ++p) {
    *p = to_lower(*p);
  }
}

void to_lower(std::string* input) {
  if (!input) {
    return;
  }
  for (char& c : *input) {
    c = to_lower(c);
  }
}

std::string to_lower(const std::string& input) {
  std::string result;
  result.resize(input.size());

  for (std::size_t i = 0; i < input.size(); ++i) {
    result[i] = to_lower(input[i]);
  }

  return result;
}

void to_upper(char* input, std::size_t len) {
  if (!input) {
    return;
  }
  for (std::size_t i = 0; i < len; i++) {
    input[i] = to_upper(input[i]);
  }
}

void to_upper(char* input) {
  if (!input) {
    return;
  }
  for (char* p = input; *p != '\0'; ++p) {
    *p = to_upper(*p);
  }
}

void to_upper(std::string* input) {
  if (!input) {
    return;
  }
  for (char& c : *input) {
    c = to_upper(c);
  }
}

std::string to_upper(const std::string& input) {
  std::string result;
  result.resize(input.size());

  for (std::size_t i = 0; i < input.size(); ++i) {
    result[i] = to_upper(input[i]);
  }

  return result;
}

std::size_t utf8_char_length(unsigned char lead) {
  if (lead < 0x80) {  // 0xxxxxxx
    return 1;
  } else if ((lead & 0xE0) == 0xC0) {  // 110xxxxx (2-byte)
    return 2;
  } else if ((lead & 0xF0) == 0xE0) {  // 1110xxxx (3-byte)
    return 3;
  } else if ((lead & 0xF8) == 0xF0) {  // 11110xxx (4-byte)
    return 4;
  } else {
    return 1;
  }
}

std::string utf8_truncate(const std::string& input, std::size_t max_chars) {
  if (max_chars == 0) {
    return "";
  }
  if (input.empty()) {
    return "";
  }

  std::size_t i = 0;      // byte index
  std::size_t chars = 0;  // character count

  const char* data = input.data();
  const std::size_t input_size = input.size();

  while (i < input_size && chars < max_chars) {
    std::size_t len = utf8_char_length(static_cast<unsigned char>(data[i]));
    // Check if the character is valid and complete within the string boundary
    if (i + len > input_size) {
      // Incomplete character at the end, break
      break;
    }
    i += len;
    ++chars;
  }

  return std::string(data, i);
}

std::queue<std::string> split_string(const std::string& input,
                                     const std::string& delimiter) {
  std::queue<std::string> result;
  std::size_t pos = 0;
  const std::size_t delimiter_length = delimiter.size();
  const std::size_t input_length = input.size();

  if (delimiter_length == 0) {
    if (!input.empty()) {
      // Each character becomes a token
      for (char c : input) {
        result.emplace(1, c);
      }
    }
    return result;
  }

  std::size_t next_pos;
  while (pos < input_length) {
    next_pos = input.find(delimiter, pos);
    if (next_pos == std::string::npos) {
      // No more delimiters, push the rest of the string
      result.push(input.substr(pos));
      break;
    } else {
      // Push the substring from pos to next_pos - pos
      result.push(input.substr(pos, next_pos - pos));
      pos = next_pos + delimiter_length;
    }
  }

  return result;
}

std::string remove_bracket(const std::string& input,
                           std::size_t max_nest_size) {
  if (input.empty()) {
    return {};
  }

  std::string output;
  output.reserve(input.size());
  output.resize(input.size());

  std::vector<char> stack(std::min(input.size() / 2, max_nest_size));
  std::size_t depth = 0;
  char* write_ptr = output.data();

  const char* read_ptr = input.data();
  const char* end_ptr = read_ptr + input.size();

  // Lookup table for bracket matching (faster than switch)
  // Index by character value, 0 means not a bracket
  static constexpr std::array<int8_t, 256> bracket_map = make_bracket_table();

  while (read_ptr < end_ptr) {
    char c = *read_ptr++;
    int8_t bracket_type = bracket_map[static_cast<unsigned char>(c)];

    if (bracket_type > 0) {
      // Opening bracket
      if (depth < max_nest_size) {
        stack[depth++] = bracket_type;
      } else {
        depth++;
      }
    } else if (bracket_type < 0) {
      // Closing bracket
      if (depth > 0) {
        if (depth <= max_nest_size && stack[depth - 1] == -bracket_type) {
          --depth;
        } else if (depth > max_nest_size) {
          --depth;
        } else {
        }
      }
    } else if (depth == 0) {
      // Not a bracket and not inside brackets
      *write_ptr++ = c;
    }
  }

  output.resize(write_ptr - output.data());
  return output;
}

std::size_t safe_strlen(const char* str) {
  if (!str) {
    return 0;
  }
  return std::strlen(str);
}

void format_address_safe(uintptr_t addr,
                         char* buffer,
                         std::size_t buffer_size) {
  if (!buffer || buffer_size < (sizeof(uintptr_t) * 2 + 3)) {
    if (buffer && buffer_size > 0) {
      buffer[0] = '\0';
    }
    return;
  }

  char* current_ptr = buffer;
  current_ptr[0] = '0';
  current_ptr[1] = 'x';
  current_ptr += 2;

  auto [ptr, ec] = std::to_chars(current_ptr, buffer + buffer_size, addr, 16);

  if (ec == std::errc{}) {
    *ptr = '\0';
  } else {
    buffer[0] = '\0';
  }
}

void padding(char*& cursor,
             const char* const end,
             std::size_t current_len,
             std::size_t align_len) {
  std::size_t pad_len = (align_len > current_len) ? align_len - current_len : 0;
  std::size_t to_pad =
      std::min(pad_len, static_cast<std::size_t>(end - cursor));
  std::memset(cursor, ' ', to_pad);
  cursor += to_pad;
}

std::size_t write_raw(char*& dest, const char* source, std::size_t len) {
  if (len == 0) {
    return 0;
  }

  std::memcpy(dest, source, len);
  return len;
}

}  // namespace femtolog::core
