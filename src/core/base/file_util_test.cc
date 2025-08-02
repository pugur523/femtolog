// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/core/base/file_util.h"

#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "femtolog/build/build_flag.h"
#include "gtest/gtest.h"

namespace femtolog::core {

TEST(FileUtilTest, TempFileLifecycle) {
  TempFile file("test_prefix_", "content");
  ASSERT_TRUE(file.valid());
  EXPECT_TRUE(file_exists(file.path().c_str()));

  std::string content = read_file(file.path().c_str());
  EXPECT_EQ(content, "content");
}

TEST(FileUtilTest, TempDirLifecycle) {
  TempDir dir("test_tmpdir_");
  ASSERT_TRUE(dir.valid());
  EXPECT_TRUE(dir_exists(dir.path().c_str()));
}

TEST(FileUtilTest, WriteAndReadFile) {
  std::string temp = temp_path("fileutil_test_");
  std::string content = "Hello, World!";
  ASSERT_EQ(write_file(temp.c_str(), content), 0);
  EXPECT_TRUE(file_exists(temp.c_str()));
  EXPECT_EQ(read_file(temp.c_str()), content);
  EXPECT_EQ(remove_file(temp.c_str()), 0);
}

TEST(FileUtilTest, WriteBinaryToFile) {
  std::string temp = temp_path("bin_test_");
  std::vector<uint8_t> data = {1, 2, 3, 4, 5};
  ASSERT_EQ(write_binary_to_file(data, temp), 0);
  EXPECT_TRUE(file_exists(temp.c_str()));

  {
    std::ifstream fin(temp, std::ios::binary);
    ASSERT_TRUE(fin);
    std::vector<char> read_data((std::istreambuf_iterator<char>(fin)),
                                std::istreambuf_iterator<char>());
    ASSERT_EQ(read_data.size(), data.size());
    for (std::size_t i = 0; i < data.size(); ++i) {
      EXPECT_EQ(static_cast<uint8_t>(read_data[i]), data[i]);
    }
  }

  int remove_result = remove_file(temp.c_str());
  EXPECT_EQ(remove_result, 0);
}

TEST(FileUtilTest, JoinPathBasic) {
  std::string result = join_path("folder", "sub", "file.txt");
#ifdef _WIN32
  EXPECT_EQ(result, "folder\\sub\\file.txt");
#else
  EXPECT_EQ(result, "folder/sub/file.txt");
#endif
}

TEST(FileUtilTest, FileExistsAndRemove) {
  std::string temp = temp_path("exist_test_");
  ASSERT_EQ(write_file(temp.c_str(), "test"), 0);
  EXPECT_TRUE(file_exists(temp.c_str()));
  EXPECT_EQ(remove_file(temp.c_str()), 0);
  EXPECT_FALSE(file_exists(temp.c_str()));
}

TEST(FileUtilTest, ReadLines) {
  std::string multiline = "line1\nline2\nline3\n";
  auto lines = read_lines<false>(multiline);
  ASSERT_EQ(lines.size(), 3);
  EXPECT_EQ(lines[0], "line1");
  EXPECT_EQ(lines[1], "line2");
  EXPECT_EQ(lines[2], "line3");
}

TEST(FileUtilTest, FileExtension) {
  EXPECT_EQ(file_extension("test.txt"), "txt");
  EXPECT_EQ(file_extension("archive.tar.gz"), "gz");
  EXPECT_EQ(file_extension("no_extension"), "");
}

}  // namespace femtolog::core
