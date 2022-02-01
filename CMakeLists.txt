cmake_minimum_required(VERSION 3.21)
project(chart-qt)

if(WASM)
    include(cmake/wasm.cmake)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/3rdparty/sanitizers-cmake/cmake" ${CMAKE_MODULE_PATH})
find_package(Sanitizers)

add_subdirectory(chart-qt)
add_subdirectory(expertui)
