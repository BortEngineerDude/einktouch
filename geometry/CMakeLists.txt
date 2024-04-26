cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0076 NEW)

set(LIBRARY_NAME geometry)
add_library(${LIBRARY_NAME})
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(${LIBRARY_NAME}

              PRIVATE
              point.cpp
              size.cpp
              rect.cpp
              line.cpp

              PUBLIC
              point.h
              size.h
              rect.h
              line.h
)
