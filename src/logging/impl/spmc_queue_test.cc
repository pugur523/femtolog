// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/spmc_queue.h"

#include <vector>

#include "gtest/gtest.h"

namespace femtolog::logging {

namespace {

class SpmcQueueTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(SpmcQueueTest, ConstructorAndCapacity) {
  SpmcQueue queue_small;
  queue_small.reserve(16);
  EXPECT_EQ(queue_small.capacity(), 16);  // Capacity is power of 2 minus 1
  EXPECT_TRUE(queue_small.empty());
  EXPECT_EQ(queue_small.size(), 0);
  EXPECT_EQ(queue_small.available_space(), 16);

  SpmcQueue queue_large;
  queue_large.reserve(1024);
  EXPECT_EQ(queue_large.capacity(), 1024);
  EXPECT_TRUE(queue_large.empty());
  EXPECT_EQ(queue_large.size(), 0);
  EXPECT_EQ(queue_large.available_space(), 1024);

  SpmcQueue queue_non_power_of_2;
  queue_non_power_of_2.reserve(20);
  EXPECT_EQ(queue_non_power_of_2.capacity(), 32);
  EXPECT_TRUE(queue_non_power_of_2.empty());
  EXPECT_EQ(queue_non_power_of_2.size(), 0);
  EXPECT_EQ(queue_non_power_of_2.available_space(), 32);
}

TEST_F(SpmcQueueTest, EnqueueDequeueSingleElement) {
  SpmcQueue queue;
  queue.reserve(16);

  int data_in = 42;
  SpmcQueueStatus result = queue.enqueue_bytes(&data_in);
  EXPECT_EQ(result, SpmcQueueStatus::kOk);
  EXPECT_FALSE(queue.empty());
  EXPECT_EQ(queue.size(), sizeof(int));
  EXPECT_EQ(queue.available_space(), 16 - sizeof(int));

  int data_out = 0;
  result = queue.dequeue_bytes(&data_out, sizeof(int));
  EXPECT_EQ(result, SpmcQueueStatus::kOk);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
  EXPECT_EQ(queue.available_space(), 16);
  EXPECT_EQ(data_out, data_in);

  char char_in = 'A';
  result = queue.enqueue_bytes(&char_in);
  EXPECT_EQ(result, SpmcQueueStatus::kOk);
  EXPECT_FALSE(queue.empty());
  EXPECT_EQ(queue.size(), sizeof(char));

  char char_out = ' ';
  result = queue.dequeue_bytes(&char_out, sizeof(char));
  EXPECT_EQ(result, SpmcQueueStatus::kOk);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(char_out, char_in);
}

TEST_F(SpmcQueueTest, FillAndOverflow) {
  SpmcQueue queue;
  queue.reserve(16);

  for (int i = 0; i < 16; ++i) {
    char data = static_cast<char>(i);
    SpmcQueueStatus result = queue.enqueue_bytes(&data);
    EXPECT_EQ(result, SpmcQueueStatus::kOk);
  }
  EXPECT_EQ(queue.size(), 16);
  EXPECT_EQ(queue.available_space(), 0);
  EXPECT_FALSE(queue.empty());

  char data_overflow = 'Z';
  SpmcQueueStatus result = queue.enqueue_bytes(&data_overflow);
  EXPECT_EQ(result, SpmcQueueStatus::kOverflow);
  EXPECT_EQ(queue.size(), 16);  // Size should remain 16
}

TEST_F(SpmcQueueTest, DequeueFromEmpty) {
  SpmcQueue queue;
  queue.reserve(16);

  int data_out = 0;
  SpmcQueueStatus result = queue.dequeue_bytes(&data_out, sizeof(int));
  EXPECT_EQ(result, SpmcQueueStatus::kUnderflow);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
}

TEST_F(SpmcQueueTest, EnqueueDequeueMixedSizes) {
  SpmcQueue queue;
  queue.reserve(64);

  int int_val_in = 12345;
  EXPECT_EQ(queue.enqueue_bytes(&int_val_in), SpmcQueueStatus::kOk);

  double double_val_in = 3.14159;
  EXPECT_EQ(queue.enqueue_bytes(&double_val_in), SpmcQueueStatus::kOk);

  struct MyStruct {
    char a;
    int16_t b;
  };

  MyStruct struct_in = {'X', 100};
  EXPECT_EQ(queue.enqueue_bytes(&struct_in), SpmcQueueStatus::kOk);

  EXPECT_EQ(queue.size(), sizeof(int) + sizeof(double) + sizeof(MyStruct));
  EXPECT_FALSE(queue.empty());

  int int_val_out = 0;
  EXPECT_EQ(queue.dequeue_bytes(&int_val_out, sizeof(int)),
            SpmcQueueStatus::kOk);
  EXPECT_EQ(int_val_out, int_val_in);

  double double_val_out = 0.0;
  EXPECT_EQ(queue.dequeue_bytes(&double_val_out, sizeof(double)),
            SpmcQueueStatus::kOk);
  EXPECT_DOUBLE_EQ(double_val_out, double_val_in);

  MyStruct struct_out;
  EXPECT_EQ(queue.dequeue_bytes(&struct_out, sizeof(MyStruct)),
            SpmcQueueStatus::kOk);
  EXPECT_EQ(struct_out.a, struct_in.a);
  EXPECT_EQ(struct_out.b, struct_in.b);

  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
}

TEST_F(SpmcQueueTest, WraparoundBehavior) {
  SpmcQueue queue;
  queue.reserve(16);

  std::vector<char> data_in(10, 'A');
  EXPECT_EQ(queue.enqueue_bytes(data_in.data(), data_in.size()),
            SpmcQueueStatus::kOk);
  EXPECT_EQ(queue.size(), 10);

  std::vector<char> data_out(5);
  EXPECT_EQ(queue.dequeue_bytes(data_out.data(), data_out.size()),
            SpmcQueueStatus::kOk);
  EXPECT_EQ(queue.size(), 5);

  std::vector<char> data_in_2(8, 'B');
  EXPECT_EQ(queue.enqueue_bytes(data_in_2.data(), data_in_2.size()),
            SpmcQueueStatus::kOk);
  EXPECT_EQ(queue.size(), 13);
  EXPECT_EQ(queue.available_space(), 16 - 13);

  std::vector<char> data_out_remaining(13);
  EXPECT_EQ(queue.dequeue_bytes(data_out_remaining.data(), 13),
            SpmcQueueStatus::kOk);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
}

TEST_F(SpmcQueueTest, EnqueueTooManyBytes) {
  SpmcQueue queue;
  queue.reserve(16);
  EXPECT_EQ(queue.capacity(), 16);
  EXPECT_TRUE(queue.empty());

  std::vector<char> large_data(20, 'X');
  SpmcQueueStatus result =
      queue.enqueue_bytes(large_data.data(), large_data.size());
  EXPECT_EQ(result, SpmcQueueStatus::kOverflow);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
}

TEST_F(SpmcQueueTest, DequeueTooManyBytes) {
  SpmcQueue queue;
  queue.reserve(16);

  std::vector<char> data_in(5, 'A');
  EXPECT_EQ(queue.enqueue_bytes(data_in.data(), data_in.size()),
            SpmcQueueStatus::kOk);
  EXPECT_EQ(queue.size(), 5);

  std::vector<char> data_out(10);
  SpmcQueueStatus result =
      queue.dequeue_bytes(data_out.data(), data_out.size());
  EXPECT_EQ(result, SpmcQueueStatus::kUnderflow);
  EXPECT_EQ(queue.size(), 5);
}

TEST_F(SpmcQueueTest, CustomStructEnqueueDequeue) {
  SpmcQueue queue;
  queue.reserve(64);

  struct TestData {
    int id;
    float value;
    char name[10];
  };

  TestData data_in = {101, 3.14f, "TestName"};
  SpmcQueueStatus result = queue.enqueue_bytes(&data_in);
  EXPECT_EQ(result, SpmcQueueStatus::kOk);
  EXPECT_EQ(queue.size(), sizeof(TestData));

  TestData data_out;
  result = queue.dequeue_bytes(&data_out, sizeof(TestData));
  EXPECT_EQ(result, SpmcQueueStatus::kOk);
  EXPECT_EQ(data_out.id, data_in.id);
  EXPECT_FLOAT_EQ(data_out.value, data_in.value);
  EXPECT_STREQ(data_out.name, data_in.name);
  EXPECT_TRUE(queue.empty());
}

}  // namespace

}  // namespace femtolog::logging

