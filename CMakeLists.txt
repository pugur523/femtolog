# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

cmake_minimum_required(VERSION 4.0.2 FATAL_ERROR)

set(PROJECT_NAME "femtolog")
set(MAIN_LIB_NAME "femtolog")
set(PROJECT_VERSION "0.1.0")
set(PROJECT_DESCRIPTION "an ultra fast logging library")
set(PROJECT_HOMEPAGE "https://twitter.com/p523n")
set(PROJECT_C_VERSION "17")
set(PROJECT_CXX_VERSION "20")
set(PROJECT_AUTHOR "pugur")
set(PROJECT_AUTHOR_CONTACT "pugurmc@gmail.com")

project(${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  DESCRIPTION ${PROJECT_DESCRIPTION}
  HOMEPAGE_URL ${PROJECT_HOMEPAGE}
  LANGUAGES C CXX
)

add_subdirectory(src)
