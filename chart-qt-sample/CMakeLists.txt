
find_package(Qt6 COMPONENTS Core Widgets Quick QuickControls2)
find_package(Qt6QmlImportScanner)

set(SOURCES main.cpp
            sindataset.cpp
            networkmodel.cpp
            devicesmodel.cpp
            qml/qml.qrc
            )

qt_add_executable(chart-qt-sample ${SOURCES})
set_property(TARGET chart-qt-sample PROPERTY CXX_STANDARD 20)
set_property(TARGET chart-qt-sample PROPERTY AUTOMOC ON)
set_property(TARGET chart-qt-sample PROPERTY AUTORCC ON)
set_property(TARGET chart-qt-sample PROPERTY AUTORCC_OPTIONS "--no-compress")
add_sanitizers(chart-qt-sample)

if(WASM)
    target_compile_definitions(chart-qt-sample PRIVATE WASM)
    target_link_libraries(chart-qt-sample PRIVATE chart-qtplugin)
endif()
target_link_libraries(chart-qt-sample PRIVATE Qt6::Quick Qt6::QuickControls2 chart-qt)
qt_import_qml_plugins(chart-qt-sample)

add_custom_target(package)
set(package_files chart-qt-sample)
set(package_dir ${CMAKE_BINARY_DIR}/package)

if (WASM)
    set(package_files chart-qt-sample.html chart-qt-sample.js chart-qt-sample.wasm qtloader.js qtlogo.svg)
endif()

foreach(ConfigFile ${package_files})
    add_custom_command(TARGET package PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${package_dir} COMMAND ${CMAKE_COMMAND} -E copy ${ConfigFile} ${package_dir})
endforeach()
