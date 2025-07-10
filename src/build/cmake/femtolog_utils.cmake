# Copyright 2025 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

string(ASCII 27 Esc)
set(ColourReset "${Esc}[m")
set(ColourBold "${Esc}[1m")
set(Gray "${Esc}[30m")
set(Red "${Esc}[31m")
set(Green "${Esc}[32m")
set(Yellow "${Esc}[33m")
set(Blue "${Esc}[34m")
set(Magenta "${Esc}[35m")
set(Cyan "${Esc}[36m")
set(White "${Esc}[37m")
set(BoldRed "${Esc}[1;31m")
set(BoldGreen "${Esc}[1;32m")
set(BoldYellow "${Esc}[1;33m")
set(BoldBlue "${Esc}[1;34m")
set(BoldMagenta "${Esc}[1;35m")
set(BoldCyan "${Esc}[1;36m")
set(BoldWhite "${Esc}[1;37m")

function(describe_target target_name)
  get_target_property(include_dirs ${target_name} INCLUDE_DIRECTORIES)
  get_target_property(compile_opts ${target_name} COMPILE_OPTIONS)
  get_target_property(compile_definitions ${target_name} COMPILE_DEFINITIONS)
  get_target_property(compile_flags ${target_name} COMPILE_FLAGS)
  get_target_property(link_opts ${target_name} LINK_OPTIONS)
  get_target_property(link_flags ${target_name} LINK_FLAGS)
  get_target_property(link_dirs ${target_name} LINK_DIRECTORIES)
  get_target_property(link_libs ${target_name} LINK_LIBRARIES)
  get_target_property(target_type ${target_name} TYPE)

  set(description "\n${Blue}-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=")

  if(include_dirs)
    set(description "${description}\n${target_name} include directories: ${include_dirs}")
  endif()

  if(compile_opts)
    set(description "${description}\n${target_name} compile options: ${compile_opts}")
  endif()

  if(compile_definitions)
    set(description "${description}\n${target_name} compile definitions: ${compile_definitions}")
  endif()

  if(compile_flags)
    set(description "${description}\n${target_name} compile flags: ${compile_flags}")
  endif()

  if(link_opts)
    set(description "${description}\n${target_name} link options: ${link_opts}")
  endif()

  if(link_flags)
    set(description "${description}\n${target_name} link flags: ${link_flags}")
  endif()

  if(link_dirs)
    set(description "${description}\n${target_name} link directories: ${link_dirs}")
  endif()

  if(link_libs)
    set(description "${description}\n${target_name} link libraries: ${link_libs}")
  endif()

  if(target_type)
    set(description "${description}\n${target_name} build type: ${target_type}")
  endif()

  set(description "${description}\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=${ColourReset}\n")

  message(${description})
endfunction()