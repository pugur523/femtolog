// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "logging/impl/spsc_queue.h"

#include <vector>

#include "gtest/gtest.h"

namespace femtolog::logging {

namespace {

class SpscQueueTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(SpscQueueTest, ConstructorAndCapacity) {
  SpscQueue queue_small;
  queue_small.reserve(16);
  EXPECT_EQ(queue_small.capacity(), 16);  // Capacity is power of 2 minus 1
  EXPECT_TRUE(queue_small.empty());
  EXPECT_EQ(queue_small.size(), 0);
  EXPECT_EQ(queue_small.available_space(), 16);

  SpscQueue queue_large;
  queue_large.reserve(1024);
  EXPECT_EQ(queue_large.capacity(), 1024);
  EXPECT_TRUE(queue_large.empty());
  EXPECT_EQ(queue_large.size(), 0);
  EXPECT_EQ(queue_large.available_space(), 1024);

  SpscQueue queue_non_power_of_2;
  queue_non_power_of_2.reserve(20);
  EXPECT_EQ(queue_non_power_of_2.capacity(), 32);
  EXPECT_TRUE(queue_non_power_of_2.empty());
  EXPECT_EQ(queue_non_power_of_2.size(), 0);
  EXPECT_EQ(queue_non_power_of_2.available_space(), 32);
}

TEST_F(SpscQueueTest, EnqueueDequeueSingleElement) {
  SpscQueue queue;
  queue.reserve(16);

  int data_in = 42;
  SpscQueueStatus result = queue.enqueue_bytes(&data_in);
  EXPECT_EQ(result, SpscQueueStatus::kOk);
  EXPECT_FALSE(queue.empty());
  EXPECT_EQ(queue.size(), sizeof(int));
  EXPECT_EQ(queue.available_space(), 16 - sizeof(int));

  int data_out = 0;
  result = queue.dequeue_bytes(&data_out, sizeof(int));
  EXPECT_EQ(result, SpscQueueStatus::kOk);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
  EXPECT_EQ(queue.available_space(), 16);
  EXPECT_EQ(data_out, data_in);

  char char_in = 'A';
  result = queue.enqueue_bytes(&char_in);
  EXPECT_EQ(result, SpscQueueStatus::kOk);
  EXPECT_FALSE(queue.empty());
  EXPECT_EQ(queue.size(), sizeof(char));

  char char_out = ' ';
  result = queue.dequeue_bytes(&char_out, sizeof(char));
  EXPECT_EQ(result, SpscQueueStatus::kOk);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(char_out, char_in);
}

TEST_F(SpscQueueTest, FillAndOverflow) {
  SpscQueue queue;
  queue.reserve(16);

  for (int i = 0; i < 16; ++i) {
    char data = static_cast<char>(i);
    SpscQueueStatus result = queue.enqueue_bytes(&data);
    EXPECT_EQ(result, SpscQueueStatus::kOk);
  }
  EXPECT_EQ(queue.size(), 16);
  EXPECT_EQ(queue.available_space(), 0);
  EXPECT_FALSE(queue.empty());

  char data_overflow = 'Z';
  SpscQueueStatus result = queue.enqueue_bytes(&data_overflow);
  EXPECT_EQ(result, SpscQueueStatus::kOverflow);
  EXPECT_EQ(queue.size(), 16);  // Size should remain 16
}

TEST_F(SpscQueueTest, DequeueFromEmpty) {
  SpscQueue queue;
  queue.reserve(16);

  int data_out = 0;
  SpscQueueStatus result = queue.dequeue_bytes(&data_out, sizeof(int));
  EXPECT_EQ(result, SpscQueueStatus::kUnderflow);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
}

TEST_F(SpscQueueTest, EnqueueDequeueMixedSizes) {
  SpscQueue queue;
  queue.reserve(64);

  int int_val_in = 12345;
  EXPECT_EQ(queue.enqueue_bytes(&int_val_in), SpscQueueStatus::kOk);

  double double_val_in = 3.14159;
  EXPECT_EQ(queue.enqueue_bytes(&double_val_in), SpscQueueStatus::kOk);

  struct MyStruct {
    char a;
    int16_t b;
  };

  MyStruct struct_in = {'X', 100};
  EXPECT_EQ(queue.enqueue_bytes(&struct_in), SpscQueueStatus::kOk);

  EXPECT_EQ(queue.size(), sizeof(int) + sizeof(double) + sizeof(MyStruct));
  EXPECT_FALSE(queue.empty());

  int int_val_out = 0;
  EXPECT_EQ(queue.dequeue_bytes(&int_val_out, sizeof(int)),
            SpscQueueStatus::kOk);
  EXPECT_EQ(int_val_out, int_val_in);

  double double_val_out = 0.0;
  EXPECT_EQ(queue.dequeue_bytes(&double_val_out, sizeof(double)),
            SpscQueueStatus::kOk);
  EXPECT_DOUBLE_EQ(double_val_out, double_val_in);

  MyStruct struct_out;
  EXPECT_EQ(queue.dequeue_bytes(&struct_out, sizeof(MyStruct)),
            SpscQueueStatus::kOk);
  EXPECT_EQ(struct_out.a, struct_in.a);
  EXPECT_EQ(struct_out.b, struct_in.b);

  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
}

TEST_F(SpscQueueTest, WraparoundBehavior) {
  SpscQueue queue;
  queue.reserve(16);

  std::vector<char> data_in(10, 'A');
  EXPECT_EQ(queue.enqueue_bytes(data_in.data(), data_in.size()),
            SpscQueueStatus::kOk);
  EXPECT_EQ(queue.size(), 10);

  std::vector<char> data_out(5);
  EXPECT_EQ(queue.dequeue_bytes(data_out.data(), data_out.size()),
            SpscQueueStatus::kOk);
  EXPECT_EQ(queue.size(), 5);

  std::vector<char> data_in_2(8, 'B');
  EXPECT_EQ(queue.enqueue_bytes(data_in_2.data(), data_in_2.size()),
            SpscQueueStatus::kOk);
  EXPECT_EQ(queue.size(), 13);
  EXPECT_EQ(queue.available_space(), 16 - 13);

  std::vector<char> data_out_remaining(13);
  EXPECT_EQ(queue.dequeue_bytes(data_out_remaining.data(), 13),
            SpscQueueStatus::kOk);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
}

TEST_F(SpscQueueTest, EnqueueTooManyBytes) {
  SpscQueue queue;
  queue.reserve(16);
  EXPECT_EQ(queue.capacity(), 16);
  EXPECT_TRUE(queue.empty());

  std::vector<char> large_data(20, 'X');
  SpscQueueStatus result =
      queue.enqueue_bytes(large_data.data(), large_data.size());
  EXPECT_EQ(result, SpscQueueStatus::kOverflow);
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0);
}

TEST_F(SpscQueueTest, DequeueTooManyBytes) {
  SpscQueue queue;
  queue.reserve(16);

  std::vector<char> data_in(5, 'A');
  EXPECT_EQ(queue.enqueue_bytes(data_in.data(), data_in.size()),
            SpscQueueStatus::kOk);
  EXPECT_EQ(queue.size(), 5);

  std::vector<char> data_out(10);
  SpscQueueStatus result =
      queue.dequeue_bytes(data_out.data(), data_out.size());
  EXPECT_EQ(result, SpscQueueStatus::kUnderflow);
  EXPECT_EQ(queue.size(), 5);
}

TEST_F(SpscQueueTest, CustomStructEnqueueDequeue) {
  SpscQueue queue;
  queue.reserve(64);

  struct TestData {
    int id;
    float value;
    char name[10];
  };

  TestData data_in = {101, 3.14f, "TestName"};
  SpscQueueStatus result = queue.enqueue_bytes(&data_in);
  EXPECT_EQ(result, SpscQueueStatus::kOk);
  EXPECT_EQ(queue.size(), sizeof(TestData));

  TestData data_out;
  result = queue.dequeue_bytes(&data_out, sizeof(TestData));
  EXPECT_EQ(result, SpscQueueStatus::kOk);
  EXPECT_EQ(data_out.id, data_in.id);
  EXPECT_FLOAT_EQ(data_out.value, data_in.value);
  EXPECT_STREQ(data_out.name, data_in.name);
  EXPECT_TRUE(queue.empty());
}

}  // namespace

}  // namespace femtolog::logging
