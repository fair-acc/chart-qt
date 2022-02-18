function(add_pipelines)
    include(CMakeParseArguments)
    cmake_parse_arguments(PIP "" "TARGET" "SHADERS;FILES" ${ARGN})

    qt6_add_shaders(${PIP_TARGET} "shaders" PREFIX "/" FILES ${PIP_SHADERS})

    foreach(s ${PIP_SHADERS})
        list(APPEND built_shaders .qsb/${s}.qsb)
    endforeach()

    foreach(f ${PIP_FILES})
        set(file ${CMAKE_CURRENT_SOURCE_DIR}/${f})
        get_filename_component(out ${file} NAME_WE)

        add_custom_command(OUTPUT ${out}.cpp ${out}.h
                   COMMAND pipegen/pipegen ARGS ${file} -s ${CMAKE_CURRENT_BINARY_DIR}/.qsb/
                   MAIN_DEPENDENCY ${file}
                   DEPENDS pipegen ${built_shaders}
                   COMMENT "Generating pipeline class for ${f}"
                   VERBATIM)
        target_sources(${PIP_TARGET} PRIVATE ${out}.cpp ${out}.h)
    endforeach()
endfunction()
