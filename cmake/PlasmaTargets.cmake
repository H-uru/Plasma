include(QtDeployment)
include(Sanitizers)

cmake_dependent_option(
    PLASMA_USE_PCH
    "Enable precompiled headers?"
    ON
    "CMAKE_VERSION VERSION_GREATER_EQUAL 3.16;ALLOW_BUILD_OPTIMIZATIONS"
    OFF
)

cmake_dependent_option(
    PLASMA_UNITY_BUILD
    "Enable unity build?"
    ON
    "CMAKE_VERSION VERSION_GREATER_EQUAL 3.16;ALLOW_BUILD_OPTIMIZATIONS"
    OFF
)

function(plasma_executable TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 _pex
        "WIN32;CLIENT;TOOL;QT_GUI;EXCLUDE_FROM_ALL;NO_SANITIZE;INSTALL_PDB"
        ""
        "SOURCES"
    )

    if(_pex_WIN32)
        list(APPEND addexe_args WIN32)
    endif()
    if(_pex_QT_GUI)
        list(APPEND addexe_args WIN32 MACOSX_BUNDLE)
    endif()
    if(_pex_CLIENT)
        list(APPEND addexe_args MACOSX_BUNDLE)
    endif()
    if(_pex_EXCLUDE_FROM_ALL)
        list(APPEND addexe_args EXCLUDE_FROM_ALL)
    endif()
    add_executable(${TARGET} ${addexe_args} ${_pex_SOURCES})
    set_target_properties(${TARGET} PROPERTIES XCODE_GENERATE_SCHEME TRUE)

    if(_pex_CLIENT)
        set(install_destination client)
    elseif(_pex_TOOL)
        add_dependencies(tools ${TARGET})
        set(install_destination tools)
    endif()

    if(_pex_QT_GUI)
        set_target_properties(${TARGET}
            PROPERTIES
                AUTOMOC ON
                AUTORCC ON
                AUTOUIC ON
        )

        target_include_directories(${TARGET}
            PRIVATE
                # Needed for custom widget includes
                ${CMAKE_CURRENT_SOURCE_DIR}
        )

        # Add to the list of tools which need windeployqt
        set_property(GLOBAL APPEND PROPERTY _PLASMA_GUI_TOOLS ${TARGET})
    endif()

    if(NOT _pex_NO_SANITIZE)
        plasma_sanitize_target(${TARGET})
    endif()

    if(DEFINED install_destination)
        install(TARGETS ${TARGET} DESTINATION ${install_destination})
        if(_pex_INSTALL_PDB AND WIN32 AND NOT MINGW)
            if(MSVC)
                set(stripped_pdb_path "$<TARGET_PDB_FILE_DIR:${TARGET}>/${TARGET}.stripped.pdb")
                target_link_options(${TARGET} PRIVATE "/PDBSTRIPPED:${stripped_pdb_path}")
                set_property(TARGET ${TARGET} APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${stripped_pdb_path})
                install(FILES
                    ${stripped_pdb_path}
                    DESTINATION ${install_destination}
                    OPTIONAL
                )
            endif()
            install(FILES
                $<TARGET_PDB_FILE:${TARGET}>
                DESTINATION ${install_destination}
                OPTIONAL
            )
        endif()
    endif()
endfunction()

function(plasma_library TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 _plib
        "UNITY_BUILD;OBJECT;SHARED;NO_SANITIZE"
        ""
        "PRECOMPILED_HEADERS;SOURCES"
    )

    if(_plib_SHARED AND _plib_OBJECT)
        message(AUTHOR_WARNING "Library ${TARGET} is both an OBJECT and SHARED library. These options are mutually exclusive.")
    endif()

    if(_plib_SHARED)
        set(libtype SHARED)
    elseif(_plib_OBJECT)
        set(libtype OBJECT)
    else()
        set(libtype STATIC)
    endif()
    add_library(${TARGET} ${libtype} ${_plib_SOURCES})

    if(PLASMA_USE_PCH AND _plib_PRECOMPILED_HEADERS)
        target_precompile_headers(${TARGET} PRIVATE ${_plib_PRECOMPILED_HEADERS})
    endif()

    if(PLASMA_UNITY_BUILD AND _plib_UNITY_BUILD)
        set_target_properties(${TARGET} PROPERTIES UNITY_BUILD TRUE)
    endif()

    if(NOT _plib_NO_SANITIZE)
        plasma_sanitize_target(${TARGET})
    endif()
endfunction()

function(plasma_test TARGET)
    plasma_executable(${TARGET} NO_SANITIZE ${ARGN})
    add_test(NAME ${TARGET} COMMAND ${TARGET})
    add_dependencies(check ${TARGET})
endfunction()
