# Based on https://stackoverflow.com/a/41199492
if(WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
    get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

    execute_process(
        COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt5_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")
    if(EXISTS ${imported_location})
        add_executable(Qt5::windeployqt IMPORTED)

        set_target_properties(
            Qt5::windeployqt PROPERTIES
            IMPORTED_LOCATION ${imported_location}
        )
    endif()
endif()

function(plasma_deploy_qt)
    include(CMakeParseArguments)
    cmake_parse_arguments(_pdqt "" "TARGET;DESTINATION" "" ${ARGN})

    if(NOT _pdqt_TARGET OR NOT _pdqt_DESTINATION)
        message(FATAL_ERROR "plasma_deploy_qt must have a valid TARGET and DESTINATION")
    endif()

    # Save some ruddy characters and install the target
    install(TARGETS ${_pdqt_TARGET} DESTINATION ${_pdqt_DESTINATION})

    if(TARGET Qt5::windeployqt)
        # Copy Qt DLLs to the build directory so the stupid application can be run in Visual Studio
        add_custom_command(
            TARGET ${_pdqt_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${_pdqt_TARGET}>/qt"
            COMMAND set PATH=%PATH%$<SEMICOLON>${qt5_install_prefix}/bin
            COMMAND Qt5::windeployqt --no-translations --no-angle --no-opengl-sw --dir "$<TARGET_FILE_DIR:${_pdqt_TARGET}>/qt" "$<TARGET_FILE:${_pdqt_TARGET}>"
        )

        # Install Qt5 DLLs for the final distribution
        install(
            DIRECTORY "$<TARGET_FILE_DIR:${_pdqt_TARGET}>/qt/"
            DESTINATION ${_pdqt_DESTINATION}
        )
    endif()
endfunction()
