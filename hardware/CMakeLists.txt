cmake_minimum_required(VERSION 3.5)

cmake_policy(SET CMP0076 NEW)

set(LIBRARY_NAME hardware)
add_library(${LIBRARY_NAME})
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${LIBRARY_NAME} PUBLIC event geometry util ui)
set_target_properties(${LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_sources(${LIBRARY_NAME}

               PRIVATE
               spi.cpp
               i2c.cpp
               gt1158.cpp
               waveshare_eink.cpp

               PUBLIC
               spi.h
               i2c.h
               gt1158.h
               waveshare_eink.h
)
