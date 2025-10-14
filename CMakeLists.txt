cmake_minimum_required(VERSION 3.28)

if(TARGET pioasm)
    return()
endif()

if(NOT CMAKE_CROSSCOMPILING)

    project(pioasm)

    include(cmake_helpers/BuildOptions.cmake)

    add_executable(pioasm src/main.cpp src/pio_assembler.cpp src/pio_disassembler.cpp src/gen/lexer.cpp
                          src/gen/parser.cpp src/kvasir_output.cpp)

    target_add_default_build_options(pioasm PRIVATE)

    target_include_directories(pioasm PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

    target_compile_options(
        pioasm
        PRIVATE -Wno-shadow
                -Wno-unused-parameter
                -Wno-sign-conversion
                -Wno-conversion
                -Wno-sign-compare
                -Wno-old-style-cast
                -Wno-missing-field-initializers
                -Wno-covered-switch-default
                -Wno-deprecated-copy-with-dtor
                -Wno-exit-time-destructors
                -Wno-extra-semi-stmt
                -Wno-format
                -Wno-global-constructors
                -Wno-missing-prototypes
                -Wno-newline-eof
                -Wno-non-virtual-dtor
                -Wno-reserved-identifier
                -Wno-reserved-macro-identifier
                -Wno-suggest-destructor-override
                -Wno-switch-enum
                -Wno-unreachable-code
                -Wno-unreachable-code-break
                -Wno-unsafe-buffer-usage
                -Wno-unused-but-set-variable
                -Wno-unused-macros
                -Wno-unused-variable
                -Wno-zero-as-null-pointer-constant
                -Wno-delete-non-abstract-non-virtual-dtor)

else()
    include(cmake_helpers/HostBuild.cmake)
    configure_host_build(pioasm)
endif()

find_program(clang-format clang-format)
mark_as_advanced(FORCE clang-format)

function(pioasm_generate targetname)

    cmake_parse_arguments(PARSE_ARGV 1 PARSED_ARGS "" "INPUT_FILE;PIO_VERSION" "")

    if(PARSED_ARGS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "unknown argument ${PARSED_ARGS_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT PARSED_ARGS_INPUT_FILE)
        message(FATAL_ERROR "pioasm needs INPUT_FILE")
    endif()

    if(NOT PARSED_ARGS_PIO_VERSION)
        set(PARSED_ARGS_PIO_VERSION 1)
    endif()

    set(iname ${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_INPUT_FILE})
    set(incname ${CMAKE_CURRENT_BINARY_DIR}/pioasm_generated)
    set(dname ${incname}/${targetname})

    get_filename_component(ofilename ${iname} NAME_WE)
    set(oname ${dname}/${ofilename}.hpp)

    get_target_property(build_dir pioasm BINARY_DIR)

    if(NOT CMAKE_CROSSCOMPILING)
        set(pioasm_bin ${build_dir}/pioasm)
    else()
        set(pioasm_bin ${build_dir}/host_build/pioasm)
    endif()

    set(command
        ${pioasm_bin}
        -o
        kvasir
        -v
        ${PARSED_ARGS_PIO_VERSION}
        ${iname}
        ${oname})

    if(NOT EXISTS ${clang-format})
        add_custom_command(
            COMMAND ${CMAKE_COMMAND} -E make_directory ${dname}
            COMMAND ${command}
            DEPENDS pioasm ${iname} ${pioasm_bin}
            OUTPUT ${oname})
    else()
        add_custom_command(
            COMMAND ${CMAKE_COMMAND} -E make_directory ${dname}
            COMMAND ${command}
            COMMAND ${clang-format} --Wno-error=unknown -i ${oname} || true
            DEPENDS pioasm ${iname} ${pioasm_bin}
            OUTPUT ${oname})
    endif()

    if(${CMAKE_VERSION} VERSION_LESS "3.19.0")
        set_source_files_properties(${oname} PROPERTIES GENERATED TRUE)
        add_custom_target(${targetname}_helper DEPENDS ${oname})
        add_library(${targetname} INTERFACE)
        add_dependencies(${targetname} ${targetname}_helper)
    else()
        add_library(${targetname} INTERFACE ${oname})
    endif()

    target_include_directories(${targetname} INTERFACE ${incname})

    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19.0")
        set_property(
            TARGET ${targetname}
            APPEND
            PROPERTY ADDITIONAL_CLEAN_FILES ${oname})
    endif()

endfunction()
