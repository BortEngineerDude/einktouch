cmake_minimum_required(VERSION 3.5)

project(einktouch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME})

cmake_policy(SET CMP0076 NEW)

target_sources(${PROJECT_NAME} PRIVATE main.cpp)

add_subdirectory(geometry)
add_subdirectory(hardware)
add_subdirectory(event)
add_subdirectory(ui)
add_subdirectory(util)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
find_package(libgpiodcxx REQUIRED)

target_link_libraries(${PROJECT_NAME}
                      PUBLIC libgpiodcxx::libgpiodcxx
                             geometry
                             hardware
                             ui
                             util
                             event
)

set(INSTALL_DESTDIR "/opt/einktouch/")

include(GNUInstallDirs)
install(TARGETS einktouch
    RUNTIME DESTINATION "${INSTALL_DESTDIR}"
    BUNDLE DESTINATION "${INSTALL_DESTDIR}"
    LIBRARY DESTINATION "${INSTALL_DESTDIR}"
)
