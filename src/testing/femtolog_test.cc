// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>

#include "femtolog/logger.h"
#include "femtolog/sinks/file_sink.h"
#include "femtolog/sinks/json_lines_sink.h"
#include "femtolog/sinks/stdout_sink.h"
#include "gtest/gtest.h"

namespace femtolog {

TEST(FemtoLogTest, BasicLogging) {
  Logger& logger = Logger::logger();

  logger.init();

  logger.register_sink<StdoutSink<>>();
  logger.register_sink<FileSink<>>();
  logger.register_sink<JsonLinesSink<>>();

  logger.start_worker();

  logger.level(LogLevel::kTrace);

  logger.info<"basic logging\n">();

  logger.info<"5 is: {}\n">(5);
  logger.warn<"this is a sample warning, 1.234 is: {}\n">(1.234);
  logger.error<"this is a sample error, true is: {}\n">(true);
  logger.debug<"this is a sample debug, false is: {}\n">(false);
  logger.trace<"this is a sample trace, string is: {}\n">("string");
  std::string msg = "message";
  logger.fatal<"this is a sample fatal {}\n">(msg);

  logger.level(LogLevel::kWarn);
  logger.trace<"this line should be invisible\n">();
  logger.debug<"this line should be invisible2\n">();
  logger.info<"this line should be invisible3\n">();
  logger.level(LogLevel::kInfo);

  logger.stop_worker();
  logger.clear_sinks();
}

}  // namespace femtolog

TEST(FemtoLogTest, README_example) {
  femtolog::Logger& logger = femtolog::Logger::logger();

  logger.init();
  logger.register_sink<femtolog::StdoutSink<>>();
  logger.register_sink<femtolog::FileSink<>>();
  logger.register_sink<femtolog::JsonLinesSink<>>();
  logger.level("trace");

  logger.start_worker();

  std::string username = "pugur";
  float cpu_usage = 42.57;
  bool result = true;
  int error_code = -1;

  logger.trace<"Hello {}\n">("World");
  logger.debug<"Hello World wo formatting\n">();
  logger.info<"User \"{}\" logged in.\n">(username);
  logger.warn<"CPU usage is high: {}%\n">(cpu_usage);
  logger.error<"Return value is: {}\n">(result);

  logger.fatal<"Fatal error occured; error code: {}\n">(error_code);

  logger.stop_worker();
  logger.clear_sinks();
}
