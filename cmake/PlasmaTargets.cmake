include(QtDeployment)
include(Sanitizers)

cmake_dependent_option(
    PLASMA_USE_PCH
    "Enable precompiled headers?"
    ON
    "CMAKE_VERSION VERSION_GREATER_EQUAL 3.16;ALLOW_BUILD_OPTIMIZATIONS"
    OFF
)

# CMake 3.28.2 breaks precompiled headers and unity builds.
# Debounce this known bad combination.
if(CMAKE_VERSION VERSION_EQUAL 3.28.2 AND PLASMA_USE_PCH AND CMAKE_GENERATOR MATCHES Ninja|Makefiles)
    set(_UNITY_BROKEN TRUE)
endif()

cmake_dependent_option(
    PLASMA_UNITY_BUILD
    "Enable unity build?"
    ON
    "CMAKE_VERSION VERSION_GREATER_EQUAL 3.16;ALLOW_BUILD_OPTIMIZATIONS;NOT _UNITY_BROKEN"
    OFF
)

if(APPLE AND NOT CMAKE_GENERATOR STREQUAL "Xcode")
    find_program(IBTOOL ibtool HINTS "/usr/bin" "${OSX_DEVELOPER_ROOT}/usr/bin")
    if(NOT IBTOOL)
        message(SEND_ERROR "Could not find Xcode's ibtool to process .xib files")
    endif()

    find_program(ACTOOL actool HINTS "/usr/bin" "${OSX_DEVELOPER_ROOT}/usr/bin")
    if(NOT ACTOOL)
        message(SEND_ERROR "Could not find Xcode's actool to process .xcasset files")
    endif()

    find_program(PLISTBUDDY PlistBuddy HINTS "/usr/libexec" "${OSX_DEVELOPER_ROOT}/usr/libexec")
    if(NOT PLISTBUDDY)
        message(SEND_ERROR "Could not find PlistBuddy for plist file processing")
    endif()
endif()

function(plasma_executable TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 _pex
        "WIN32;CLIENT;TOOL;QT_GUI;EXCLUDE_FROM_ALL;NO_SANITIZE;INSTALL_PDB"
        "FOLDER"
        "SOURCES"
    )

    if(_pex_WIN32)
        list(APPEND addexe_args WIN32)
    endif()
    if(_pex_QT_GUI OR _pex_CLIENT)
        list(APPEND addexe_args WIN32 MACOSX_BUNDLE)
    endif()
    if(_pex_EXCLUDE_FROM_ALL)
        list(APPEND addexe_args EXCLUDE_FROM_ALL)
    endif()
    add_executable(${TARGET} ${addexe_args} ${_pex_SOURCES})
    set_target_properties(${TARGET} PROPERTIES XCODE_GENERATE_SCHEME TRUE)

    if(_pex_FOLDER)
        set_target_properties(${TARGET} PROPERTIES FOLDER ${_pex_FOLDER})
    endif()

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

    # Xcode will automatically run ibtool to compile the XIB files into NIB
    # resources, but if we're generating Makefiles or Ninja projects then we
    # need to handle that ourselves...
    if(APPLE AND _pex_CLIENT)
        foreach(SRCFILE IN LISTS _pex_SOURCES)
            get_filename_component(SRCEXTENSION ${SRCFILE} LAST_EXT)

            if(CMAKE_GENERATOR STREQUAL "Xcode" AND (${SRCEXTENSION} STREQUAL ".xib" OR ${SRCEXTENSION} STREQUAL ".xcassets"))
                set_source_files_properties(${SRCFILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
            elseif(${SRCEXTENSION} STREQUAL ".xib")
                set_source_files_properties(${SRCFILE} PROPERTIES HEADER_FILE_ONLY ON)
                get_filename_component(XIBFILENAME ${SRCFILE} NAME_WE)
                add_custom_command(TARGET ${TARGET} POST_BUILD
                    COMMAND ${IBTOOL} --output-format human-readable-text --compile "$<TARGET_BUNDLE_CONTENT_DIR:${TARGET}>/Resources/${XIBFILENAME}.nib" "${CMAKE_CURRENT_SOURCE_DIR}/${SRCFILE}"
                    COMMENT "Compiling ${SRCFILE} to ${XIBFILENAME}.nib"
                    VERBATIM
                )
            elseif(${SRCEXTENSION} STREQUAL ".xcassets")
                set_source_files_properties(${SRCFILE} PROPERTIES HEADER_FILE_ONLY ON)
                get_filename_component(XACFILENAME ${SRCFILE} NAME_WE)
                add_custom_command(TARGET ${TARGET} POST_BUILD
                    COMMAND ${ACTOOL} --output-format human-readable-text --notices --warnings --output-partial-info-plist "${XACFILENAME}.plist" --app-icon AppIcon --enable-on-demand-resources NO --development-region en --target-device mac --minimum-deployment-target ${CMAKE_OSX_DEPLOYMENT_TARGET} --platform macosx --compile "$<TARGET_BUNDLE_CONTENT_DIR:${TARGET}>/Resources" "${CMAKE_CURRENT_SOURCE_DIR}/${SRCFILE}"
                    COMMENT "Processing ${SRCFILE}..."
                    VERBATIM
                )
                add_custom_command(TARGET ${TARGET} POST_BUILD
                    COMMAND ${PLISTBUDDY} -c "Clear dict" -c "Merge ${XACFILENAME}.plist" -c "Merge $<TARGET_BUNDLE_CONTENT_DIR:${TARGET}>/Info.plist" "$<TARGET_BUNDLE_CONTENT_DIR:${TARGET}>/Info.plist"
                    COMMENT "Merging Info.plist..."
                    VERBATIM
                )
            endif()
        endforeach()
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
        "FOLDER"
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

    if(_plib_FOLDER)
        set_target_properties(${TARGET} PROPERTIES FOLDER ${_plib_FOLDER})
    endif()

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
    plasma_executable(${TARGET} NO_SANITIZE FOLDER Tests ${ARGN})
    add_test(NAME ${TARGET} COMMAND ${TARGET})
    add_dependencies(check ${TARGET})
endfunction()
