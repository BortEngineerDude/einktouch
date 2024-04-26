cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0076 NEW)

set(LIBRARY_NAME event)
add_library(${LIBRARY_NAME})
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_sources(${LIBRARY_NAME}

               PUBLIC
               touch.h

               PRIVATE
               touch.cpp
)
