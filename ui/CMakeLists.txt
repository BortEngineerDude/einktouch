cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0076 NEW)

set(LIBRARY_NAME ui)
add_library(${LIBRARY_NAME})
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${LIBRARY_NAME} PUBLIC geometry util event)

target_sources(${LIBRARY_NAME} PRIVATE
  element.cpp
  bitmap.cpp
  bmp_image.cpp
  painter.cpp
  button.cpp
)

target_sources(${LIBRARY_NAME} PUBLIC
  element.h
  bitmap.h
  bmp_image.h
  painter.h
  button.h
)
