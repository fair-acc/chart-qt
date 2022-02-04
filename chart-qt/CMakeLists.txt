
find_package(Qt6 COMPONENTS Core Quick ShaderTools)

set(SOURCES chartitem.cpp
            axis.cpp
            dataset.cpp
            plot.cpp
            xyplot.cpp
            waterfallplot.cpp
            defaultchartinputhandler.cpp
            chartlayout.cpp
            )

qt_add_library(chart-qt ${SOURCES})

set_property(TARGET chart-qt PROPERTY CXX_STANDARD 20)
set_property(TARGET chart-qt PROPERTY AUTOMOC ON)
add_sanitizers(chart-qt)

qt6_add_qml_module(chart-qt
    URI ChartQt
    VERSION 1.0
)

if(WASM)
    target_compile_definitions(chart-qt PRIVATE WASM)
endif()
target_include_directories(chart-qt PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS})
target_include_directories(chart-qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(chart-qt PUBLIC Qt6::Quick)

qt6_add_shaders(chart-qt "shaders" PREFIX "/" FILES
    "shaders/xyplot_float.vert"
    "shaders/xyplot_float.frag"
    "shaders/xyplot_errorbars.vert"
    "shaders/xyplot_errorbars.frag"
    "shaders/waterfall.vert"
    "shaders/waterfall.frag"
)

qt6_add_shaders(chart-qt "qqshaders" PREFIX "/"  BATCHABLE FILES
    "shaders/axismaterial.vert"
    "shaders/axismaterial.frag"
)
