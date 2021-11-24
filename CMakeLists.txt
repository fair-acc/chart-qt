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
find_package(Qt6 COMPONENTS Core Widgets Quick QuickControls2 OpenGL ShaderTools)

find_package(Qt6QmlImportScanner REQUIRED)

set(SOURCES src/main.cpp
            src/chartitem.cpp
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
target_include_directories(chart-qt PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS})
target_link_libraries(chart-qt PRIVATE Qt6::Quick Qt6::QuickControls2)
qt_import_qml_plugins(chart-qt)

qt6_add_shaders(chart-qt "shaders" PREFIX "/" FILES
    "src/shaders/xyplot_float.vert"
    "src/shaders/xyplot_float.frag"
    "src/shaders/waterfall.vert"
    "src/shaders/waterfall.frag"
)

if(WASM)
    link_qt_static(chart-qt)
endif()
