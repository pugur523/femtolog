# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

add_library(femtolog_headers INTERFACE)
target_include_directories(femtolog_headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${FEMTOLOG_INCLUDE_DIRECTORIES})

if(FEMTOLOG_INSTALL_HEADERS)
  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include
    COMPONENT Headers
  )
endif()
