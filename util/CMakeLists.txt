cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0076 NEW)

add_library(util INTERFACE)

set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(util INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_sources(util INTERFACE
  byte_util.h
  math_util.h
)
