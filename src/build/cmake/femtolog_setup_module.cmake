# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

function(femtolog_setup_module module_name objects_name)
  set(multi_value_args INCLUDE_DIRS LINK_DIRS COMPILE_OPTIONS LINK_OPTIONS LINK_LIBS COMPILE_DEFINITIONS)
  cmake_parse_arguments(ARG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  string(TOUPPER ${module_name} UPPER_MODULE_NAME)

  target_compile_definitions(${objects_name} PRIVATE FEMTOLOG_IS_${UPPER_MODULE_NAME}_IMPL=1)

  if(ARG_INCLUDE_DIRS)
    target_include_directories(${objects_name} PRIVATE ${ARG_INCLUDE_DIRS})
  endif()

  if(ARG_COMPILE_OPTIONS)
    target_compile_options(${objects_name} PRIVATE ${ARG_COMPILE_OPTIONS})
  endif()

  if(ARG_COMPILE_DEFINITIONS)
    target_compile_definitions(${objects_name} PRIVATE ${ARG_COMPILE_DEFINITIONS})
  endif()

  if(ARG_LINK_OPTIONS)
    target_link_options(${objects_name} PRIVATE ${ARG_LINK_OPTIONS})
  endif()

  if(ARG_LINK_DIRS)
    target_link_directories(${objects_name} PRIVATE ${ARG_LINK_DIRS})
  endif()

  if(ARG_LINK_LIBS)
    target_link_libraries(${objects_name} PRIVATE ${ARG_LINK_LIBS})
  endif()

  set_target_properties(${objects_name} PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
  )

  set(build_option_name BUILD_${UPPER_MODULE_NAME}_SHARED)
  option(${build_option_name} "build ${module_name} as shared lib" ${FEMTOLOG_BUILD_SHARED})

  if(${${build_option_name}})
    add_library(${module_name} SHARED $<TARGET_OBJECTS:${objects_name}>)
    target_compile_definitions(${module_name} PUBLIC FEMTOLOG_IS_${UPPER_MODULE_NAME}_STATIC=0)
  else()
    add_library(${module_name} STATIC $<TARGET_OBJECTS:${objects_name}>)
    target_compile_definitions(${module_name} PUBLIC FEMTOLOG_IS_${UPPER_MODULE_NAME}_STATIC=1)
  endif()

  if(ARG_INCLUDE_DIRS)
    target_include_directories(${module_name} PRIVATE ${ARG_INCLUDE_DIRS})
  endif()

  if(ARG_COMPILE_OPTIONS)
    target_compile_options(${module_name} PRIVATE ${ARG_COMPILE_OPTIONS})
  endif()

  if(ARG_COMPILE_DEFINITIONS)
    target_compile_definitions(${module_name} PRIVATE ${ARG_COMPILE_DEFINITIONS})
  endif()

  if(ARG_LINK_OPTIONS)
    target_link_options(${module_name} PRIVATE ${ARG_LINK_OPTIONS})
  endif()

  if(ARG_LINK_DIRS)
    target_link_directories(${module_name} PRIVATE ${ARG_LINK_DIRS})
  endif()

  if(ARG_LINK_LIBS)
    target_link_libraries(${module_name} PRIVATE ${ARG_LINK_LIBS})
  endif()

  set_target_properties(${module_name} PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
  )

  if(FEMTOLOG_DO_CLANG_TIDY)
    set_target_properties(${objects_name} PROPERTIES
      CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
    )
  endif()

  set(install_option_name INSTALL_${UPPER_MODULE_NAME})
  option(${install_option_name} "install ${module_name}" FALSE)

  if(${FEMTOLOG_INSTALL_LIBS} AND ${${install_option_name}})
    install(
      TARGETS ${module_name}
      RUNTIME
      COMPONENT Runtime
      ARCHIVE
      COMPONENT Archive
      LIBRARY
      COMPONENT Library
    )

    install(
      FILES "$<TARGET_FILE_DIR:${module_name}>/${module_name}.pdb" "$<TARGET_FILE_DIR:${module_name}>/${module_name}d.pdb"
      DESTINATION bin
      COMPONENT Runtime
      OPTIONAL
    )
  endif()
endfunction()
