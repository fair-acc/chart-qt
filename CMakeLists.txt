cmake_minimum_required(VERSION 3.21)
project(chart-qt)

if(WASM)
    include(cmake/wasm.cmake)
endif()

include(FetchContent)

FetchContent_Declare(sanitizers GIT_REPOSITORY https://github.com/arsenm/sanitizers-cmake.git)
FetchContent_Populate(sanitizers)

set(CMAKE_MODULE_PATH "${sanitizers_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(Sanitizers)

add_subdirectory(chart-qt)
add_subdirectory(expertui)
