// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "logging/impl/internal_logger.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "femtolog/base/log_entry.h"
#include "femtolog/base/log_level.h"
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"

namespace femtolog::logging {
using ::testing::_;
using ::testing::AtLeast;
using ::testing::StrictMock;

// Mock Sink class for testing
class MockSink : public SinkBase {
 public:
  MockSink() = default;
  ~MockSink() override = default;

  MOCK_METHOD(void,
              on_log,
              (const LogEntry& entry, const char* content, std::size_t len),
              (override));

  // Helper method to capture log data
  struct LogData {
    uint32_t thread_id;
    uint16_t format_id;
    LogLevel level;
    std::string message;
    uint64_t timestamp_ns;
  };

  std::vector<LogData> captured_logs;

  void capture_log(const LogEntry& entry,
                   const char* content,
                   std::size_t len) {
    LogData data;
    data.thread_id = entry.thread_id;
    data.format_id = entry.format_id;
    data.level = entry.level;
    data.message = std::string(content, len);
    data.timestamp_ns = entry.timestamp_ns;
    captured_logs.push_back(data);
  }
};

class InternalLoggerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    logger_ = std::make_unique<InternalLogger>();
    mock_sink_ = std::make_unique<StrictMock<MockSink>>();
    mock_sink_ptr_ = mock_sink_.get();
  }

  void TearDown() override {}

  std::unique_ptr<InternalLogger> logger_;
  std::unique_ptr<StrictMock<MockSink>> mock_sink_;
  MockSink* mock_sink_ptr_;
};

// Test basic initialization
TEST_F(InternalLoggerTest, BasicInitialization) {
  logger_->init();
  EXPECT_EQ(logger_->level(), LogLevel::kInfo);
  EXPECT_EQ(logger_->enqueued_count(), 0);
  EXPECT_EQ(logger_->dropped_count(), 0);
  EXPECT_NE(logger_->thread_id(), 0);
}

// Test initialization with custom options
TEST_F(InternalLoggerTest, InitializationWithOptions) {
  FemtologOptions options;
  // Assuming FemtologOptions has log level setting
  logger_->init(options);
}

// Test sink registration
TEST_F(InternalLoggerTest, SinkRegistration) {
  logger_->init();

  logger_->register_sink(std::move(mock_sink_));

  // mock_sink_ is now nullptr after move
  EXPECT_EQ(mock_sink_.get(), nullptr);
}

// Test clearing sinks
TEST_F(InternalLoggerTest, ClearSinks) {
  logger_->init();
  logger_->register_sink(std::move(mock_sink_));

  logger_->clear_sinks();
}

// Test worker start/stop
TEST_F(InternalLoggerTest, WorkerStartStop) {
  logger_->init();
  logger_->register_sink(std::move(mock_sink_));

  logger_->start_worker();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  logger_->stop_worker();
}

// Test log level setting
TEST_F(InternalLoggerTest, LogLevelSetting) {
  logger_->init();

  logger_->level(LogLevel::kDebug);
  EXPECT_EQ(logger_->level(), LogLevel::kDebug);

  logger_->level(LogLevel::kError);
  EXPECT_EQ(logger_->level(), LogLevel::kError);
}

// Test literal string logging
TEST_F(InternalLoggerTest, LiteralStringLogging) {
  logger_->init();

  // Set up mock expectations
  EXPECT_CALL(*mock_sink_ptr_, on_log(_, _, _))
      .WillOnce(
          [this](const LogEntry& entry, const char* content, std::size_t len) {
            mock_sink_ptr_->capture_log(entry, content, len);
          });

  logger_->register_sink(std::move(mock_sink_));
  logger_->start_worker();

  // Log a simple message
  constexpr FixedString msg = "Test message";
  bool result = logger_->log<LogLevel::kInfo, msg>();
  EXPECT_TRUE(result);

  // Wait for processing
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  logger_->stop_worker();

  // Verify captured data
  ASSERT_EQ(mock_sink_ptr_->captured_logs.size(), 1);
  const auto& log_data = mock_sink_ptr_->captured_logs[0];
  EXPECT_EQ(log_data.level, LogLevel::kInfo);
  EXPECT_EQ(log_data.format_id, InternalLogger::kLiteralLogFormatId);
  EXPECT_EQ(log_data.message, "Test message");
  EXPECT_EQ(log_data.thread_id, logger_->thread_id());
}

// Test parameterized logging
TEST_F(InternalLoggerTest, ParameterizedLogging) {
  logger_->init();

  EXPECT_CALL(*mock_sink_ptr_, on_log(_, _, _))
      .WillOnce(
          [this](const LogEntry& entry, const char* content, std::size_t len) {
            mock_sink_ptr_->capture_log(entry, content, len);
          });

  logger_->register_sink(std::move(mock_sink_));
  logger_->start_worker();

  // Log with parameters
  constexpr FixedString fmt = "Value: {}";
  bool result = logger_->log<LogLevel::kInfo, fmt>(42);
  EXPECT_TRUE(result);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  logger_->stop_worker();

  // Verify captured data
  ASSERT_EQ(mock_sink_ptr_->captured_logs.size(), 1);
  const auto& log_data = mock_sink_ptr_->captured_logs[0];
  EXPECT_EQ(log_data.level, LogLevel::kInfo);
  EXPECT_NE(log_data.format_id, InternalLogger::kLiteralLogFormatId);
  EXPECT_EQ(log_data.thread_id, logger_->thread_id());
}

// Test log level filtering
TEST_F(InternalLoggerTest, LogLevelFiltering) {
  logger_->init();
  logger_->level(LogLevel::kWarn);  // Set threshold to Warning

  logger_->register_sink(std::move(mock_sink_));
  logger_->start_worker();

  // This should be filtered out (Debug < Warn)
  constexpr FixedString debug_msg = "Debug message";
  bool debug_result = logger_->log<LogLevel::kDebug, debug_msg>();
  EXPECT_FALSE(debug_result);

  // This should pass through (Error > Warn)
  EXPECT_CALL(*mock_sink_ptr_, on_log(_, _, _))
      .WillOnce(
          [this](const LogEntry& entry, const char* content, std::size_t len) {
            mock_sink_ptr_->capture_log(entry, content, len);
          });

  constexpr FixedString error_msg = "Error message";
  bool error_result = logger_->log<LogLevel::kError, error_msg>();
  EXPECT_TRUE(error_result);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  logger_->stop_worker();

  // Only error message should be captured
  ASSERT_EQ(mock_sink_ptr_->captured_logs.size(), 1);
  EXPECT_EQ(mock_sink_ptr_->captured_logs[0].level, LogLevel::kError);
  EXPECT_EQ(mock_sink_ptr_->captured_logs[0].message, "Error message");
}

// Test multiple sinks
TEST_F(InternalLoggerTest, MultipleSinks) {
  logger_->init();

  auto mock_sink1 = std::make_unique<StrictMock<MockSink>>();
  auto mock_sink2 = std::make_unique<StrictMock<MockSink>>();
  auto* sink1_ptr = mock_sink1.get();
  auto* sink2_ptr = mock_sink2.get();

  EXPECT_CALL(*sink1_ptr, on_log(_, _, _))
      .WillOnce([sink1_ptr](const LogEntry& entry, const char* content,
                            std::size_t len) {
        sink1_ptr->capture_log(entry, content, len);
      });

  EXPECT_CALL(*sink2_ptr, on_log(_, _, _))
      .WillOnce([sink2_ptr](const LogEntry& entry, const char* content,
                            std::size_t len) {
        sink2_ptr->capture_log(entry, content, len);
      });

  logger_->register_sink(std::move(mock_sink1));
  logger_->register_sink(std::move(mock_sink2));
  logger_->start_worker();

  constexpr FixedString msg = "Multi-sink test";
  bool result = logger_->log<LogLevel::kInfo, msg>();
  EXPECT_TRUE(result);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  logger_->stop_worker();

  // Both sinks should receive the message
  ASSERT_EQ(sink1_ptr->captured_logs.size(), 1);
  ASSERT_EQ(sink2_ptr->captured_logs.size(), 1);
  EXPECT_EQ(sink1_ptr->captured_logs[0].message, "Multi-sink test");
  EXPECT_EQ(sink2_ptr->captured_logs[0].message, "Multi-sink test");
}

// Test enqueue/drop counters
TEST_F(InternalLoggerTest, EnqueueDropCounters) {
  logger_->init();
  logger_->register_sink(std::move(mock_sink_));
  logger_->start_worker();

  uint64_t initial_enqueued = logger_->enqueued_count();
  uint64_t initial_dropped = logger_->dropped_count();

  EXPECT_CALL(*mock_sink_ptr_, on_log(_, _, _))
      .WillOnce(
          [this](const LogEntry& entry, const char* content, std::size_t len) {
            mock_sink_ptr_->capture_log(entry, content, len);
          });

  constexpr FixedString msg = "Counter test";
  bool result = logger_->log<LogLevel::kInfo, msg>();
  EXPECT_TRUE(result);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  logger_->stop_worker();

  // Enqueued count should have increased
  EXPECT_GT(logger_->enqueued_count(), initial_enqueued);
  // Dropped count should remain the same for successful logging
  EXPECT_EQ(logger_->dropped_count(), initial_dropped);
}

// Test different log levels
TEST_F(InternalLoggerTest, DifferentLogLevels) {
  logger_->init();
  logger_->level(LogLevel::kTrace);  // Allow all levels

  EXPECT_CALL(*mock_sink_ptr_, on_log(_, _, _))
      .Times(6)
      .WillRepeatedly(
          [this](const LogEntry& entry, const char* content, std::size_t len) {
            mock_sink_ptr_->capture_log(entry, content, len);
          });

  logger_->register_sink(std::move(mock_sink_));
  logger_->start_worker();

  // Log at different levels
  constexpr FixedString trace_msg = "Trace";
  constexpr FixedString debug_msg = "Debug";
  constexpr FixedString info_msg = "Info";
  constexpr FixedString warn_msg = "Warn";
  constexpr FixedString error_msg = "Error";
  constexpr FixedString fatal_msg = "Fatal";

  logger_->log<LogLevel::kTrace, trace_msg>();
  logger_->log<LogLevel::kDebug, debug_msg>();
  logger_->log<LogLevel::kInfo, info_msg>();
  logger_->log<LogLevel::kWarn, warn_msg>();
  logger_->log<LogLevel::kError, error_msg>();
  logger_->log<LogLevel::kFatal, fatal_msg>();

  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  logger_->stop_worker();

  // Verify all levels were captured
  ASSERT_EQ(mock_sink_ptr_->captured_logs.size(), 6);

  std::vector<LogLevel> expected_levels = {LogLevel::kTrace, LogLevel::kDebug,
                                           LogLevel::kInfo,  LogLevel::kWarn,
                                           LogLevel::kError, LogLevel::kFatal};

  for (std::size_t i = 0; i < expected_levels.size(); ++i) {
    EXPECT_EQ(mock_sink_ptr_->captured_logs[i].level, expected_levels[i]);
  }
}

// Test thread ID consistency
TEST_F(InternalLoggerTest, ThreadIdConsistency) {
  logger_->init();

  EXPECT_CALL(*mock_sink_ptr_, on_log(_, _, _))
      .WillOnce(
          [this](const LogEntry& entry, const char* content, std::size_t len) {
            mock_sink_ptr_->capture_log(entry, content, len);
          });

  logger_->register_sink(std::move(mock_sink_));
  logger_->start_worker();

  uint32_t logger_thread_id = logger_->thread_id();

  constexpr FixedString msg = "Thread ID test";
  logger_->log<LogLevel::kInfo, msg>();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  logger_->stop_worker();

  ASSERT_EQ(mock_sink_ptr_->captured_logs.size(), 1);
  EXPECT_EQ(mock_sink_ptr_->captured_logs[0].thread_id, logger_thread_id);
}

// Integration test with realistic scenario
TEST_F(InternalLoggerTest, RealisticLoggingScenario) {
  logger_->init();
  logger_->level(LogLevel::kDebug);

  EXPECT_CALL(*mock_sink_ptr_, on_log(_, _, _))
      .Times(AtLeast(3))
      .WillRepeatedly(
          [this](const LogEntry& entry, const char* content, std::size_t len) {
            mock_sink_ptr_->capture_log(entry, content, len);
          });

  logger_->register_sink(std::move(mock_sink_));
  logger_->start_worker();

  // Simulate realistic logging scenario
  constexpr FixedString startup_msg = "Application starting";
  constexpr FixedString process_fmt = "Processing item: {}";
  constexpr FixedString complete_msg = "Processing complete";

  // Log startup
  logger_->log<LogLevel::kInfo, startup_msg>();

  // Log some processing
  for (int i = 0; i < 3; ++i) {
    logger_->log<LogLevel::kDebug, process_fmt>(i);
  }

  // Log completion
  logger_->log<LogLevel::kInfo, complete_msg>();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  logger_->stop_worker();

  // Verify we got the expected logs
  EXPECT_GE(mock_sink_ptr_->captured_logs.size(), 3);

  // Check that timestamps are reasonable (increasing)
  if (mock_sink_ptr_->captured_logs.size() > 1) {
    for (std::size_t i = 1; i < mock_sink_ptr_->captured_logs.size(); ++i) {
      EXPECT_GE(mock_sink_ptr_->captured_logs[i].timestamp_ns,
                mock_sink_ptr_->captured_logs[i - 1].timestamp_ns);
    }
  }
}

}  // namespace femtolog::logging
