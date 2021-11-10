cmake_minimum_required(VERSION 3.21)
project(chart-qt)

if(WASM)
    include(cmake/wasm.cmake)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/3rdparty/sanitizers-cmake/cmake" ${CMAKE_MODULE_PATH})

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if (NOT SANITIZE_ADDRESS AND NOT SANITIZE_UNDEFINED AND NOT SANITIZE_LINK_STATIC AND NOT SANITIZE_MEMORY AND NOT SANITIZE_THREAD)
        message("Enabling sanitizers")
        option(SANITIZE_ADDRESS "" ON)
        option(SANITIZE_UNDEFINED "" ON)
    endif()
endif()

find_package(Sanitizers)
find_package(Qt6 COMPONENTS Core Widgets Quick QuickControls2 OpenGL Quick3D)

find_package(Qt6QmlImportScanner REQUIRED)

set(SOURCES src/main.cpp
            src/dataset.cpp
            src/sindataset.cpp
            src/plot.cpp
            src/xyplot.cpp
            src/waterfallplot.cpp
            src/qml/qml.qrc
            )

add_executable(chart-qt ${SOURCES})
set_property(TARGET chart-qt PROPERTY CXX_STANDARD 20)
set_property(TARGET chart-qt PROPERTY AUTOMOC ON)
set_property(TARGET chart-qt PROPERTY AUTORCC ON)
add_sanitizers(chart-qt)

if(WASM)
    target_compile_definitions(chart-qt PRIVATE WASM)
endif()
target_include_directories(chart-qt PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(chart-qt PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Quick3D)
qt_import_qml_plugins(chart-qt)

if(WASM)
    link_qt_static(chart-qt)
endif()
