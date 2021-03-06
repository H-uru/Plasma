find_package(Qt5 COMPONENTS Core Widgets)

# Based on https://stackoverflow.com/a/41199492 for easily deploying Qt DLLs on Windows.
if(WIN32 AND TARGET Qt5::qmake AND NOT TARGET Qt5::windeployqt)
    get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

    execute_process(
        COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt5_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(EXISTS "${qt5_install_prefix}/bin/windeployqt.exe")
        add_executable(Qt5::windeployqt IMPORTED)

        set_target_properties(
            Qt5::windeployqt PROPERTIES
            IMPORTED_LOCATION "${qt5_install_prefix}/bin/windeployqt.exe"
        )
    endif()
endif()

# Allow opting into all this crazy Qt copying mess.
cmake_dependent_option(PLASMA_APPLOCAL_QT "Deploy Qt libraries into tool binary directories" OFF "TARGET Qt5::windeployqt" OFF)
cmake_dependent_option(PLASMA_INSTALL_QT "Deploy Qt libraries when installing Plasma" ON "TARGET Qt5::windeployqt" OFF)

function(plasma_deploy_qt)
    # Need to deploy Qt to the binary directory for each tool so it can be debugged more easily
    # in the IDE (this is analagous to VCPKG_APPLOCAL_DEPS). Note: add_custom_command cannot
    # attach to targets in a subdir.
    get_property(gui_tools GLOBAL PROPERTY _PLASMA_GUI_TOOLS)
    foreach(i IN LISTS gui_tools)
        if(PLASMA_APPLOCAL_QT)
            add_custom_target(
                ${i}_deployqt ALL
                COMMAND Qt5::windeployqt --no-translations --no-angle --no-opengl-sw --dir "$<TARGET_FILE_DIR:${i}>" "$<TARGET_FILE:${i}>"
                WORKING_DIRECTORY "$<TARGET_FILE_DIR:Qt5::windeployqt>"
            )
            add_dependencies(${i}_deployqt ${i})
        endif()
        list(APPEND _INSTALL_DEPLOY "$<TARGET_FILE:${i}>")
    endforeach()

    # Deploy only once on install
    if(PLASMA_INSTALL_QT)
        string(JOIN [[" "]] _DEPLOY_ARG ${_INSTALL_DEPLOY})
        install(
            CODE
            "execute_process(
                COMMAND \"$<TARGET_FILE:Qt5::windeployqt>\" --no-translations --no-angle --no-opengl-sw --dir \"${CMAKE_INSTALL_PREFIX}/tools\" \"${_DEPLOY_ARG}\"
                WORKING_DIRECTORY \"$<TARGET_FILE_DIR:Qt5::windeployqt>\"
                COMMAND_ERROR_IS_FATAL ANY
            )"
        )
    endif()
endfunction()
