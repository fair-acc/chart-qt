
find_package(Qt6 COMPONENTS Core Quick ShaderTools)

# pipegen is compiled as an external library because simply using add_subdirectory()
# does not work when building for wasm, as it would also be built for wasm while we
# instead need it to be built for the host since it's only used at build time
# This has the downside that changing its sources will not trigger an automatic
# rebuild, but for now this is good enough.
include(ExternalProject)
ExternalProject_Add(pipegen
                    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pipegen
                    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/pipegen
                    INSTALL_COMMAND ""
                    CMAKE_ARGS
                        "-DCMAKE_PREFIX_PATH=${QT_HOST_PATH}")

# This shaders library is to make sure the shaders get compiled before
# the pipegen tool needs them
# A source file is needed so create an empty one
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/shaders_dummy.cpp "")
qt_add_library(shaders STATIC ${CMAKE_CURRENT_BINARY_DIR}/shaders_dummy.cpp)
qt6_add_shaders(shaders "shaders" PREFIX "/" FILES
    "shaders/xyplot_float.vert"
    "shaders/xyplot_float.frag"
    "shaders/xyplot_errorbars.vert"
    "shaders/xyplot_errorbars.frag"
    "shaders/waterfall.vert"
    "shaders/waterfall.frag"
)

function(add_pipelines)
    set(singleValues TARGET)
    set(multiValues SOURCES FILES)

    include(CMakeParseArguments)
    cmake_parse_arguments(PIP "" "TARGET" "FILES" ${ARGN})

    foreach(f ${PIP_FILES})
        set(file ${CMAKE_CURRENT_SOURCE_DIR}/${f})
        get_filename_component(out ${file} NAME_WE)

        add_custom_command(OUTPUT ${out}.cpp ${out}.h
                   COMMAND pipegen/pipegen ARGS ${file} -s ${CMAKE_CURRENT_BINARY_DIR}/.qsb/
                   MAIN_DEPENDENCY ${file}
                   DEPENDS pipegen
                   COMMENT "Generating pipeline class for ${f}"
                   VERBATIM)
        target_sources(${PIP_TARGET} PRIVATE ${out}.cpp ${out}.h)
    endforeach()
endfunction()

set(SOURCES chartitem.cpp
            axis.cpp
            dataset.cpp
            plot.cpp
            xyplot.cpp
            waterfallplot.cpp
            defaultchartinputhandler.cpp
            chartlayout.cpp
            renderutils.cpp
            )

qt_add_library(chart-qt ${SOURCES})

set_property(TARGET chart-qt PROPERTY CXX_STANDARD 20)
set_property(TARGET chart-qt PROPERTY AUTOMOC ON)
add_sanitizers(chart-qt)

qt6_add_shaders(chart-qt "qqshaders" PREFIX "/"  BATCHABLE FILES
    "shaders/axismaterial.vert"
    "shaders/axismaterial.frag"
)

add_pipelines(TARGET chart-qt
              FILES shaders/xyplotpipeline.json
                    shaders/errorbarspipeline.json
                    shaders/waterfallpipeline.json)


qt6_add_qml_module(chart-qt
    URI ChartQt
    VERSION 1.0
)

if(WASM)
    target_compile_definitions(chart-qt PRIVATE WASM)
endif()
target_include_directories(chart-qt PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS})
target_include_directories(chart-qt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(chart-qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(chart-qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/function_ref/include)
target_link_libraries(chart-qt PUBLIC Qt6::Quick shaders)
