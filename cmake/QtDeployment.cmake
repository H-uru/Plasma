find_package(Qt6 QUIET COMPONENTS Core Widgets)
set(Qt_FOUND ${Qt6_FOUND})
set(_qt_qmake_target Qt6::qmake)

if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets)
    set(Qt_FOUND ${Qt5_FOUND})
    set(_qt_qmake_target Qt5::qmake)

    if(Qt5_FOUND AND NOT TARGET Qt::Core)
        # The version-generic targets were only added in Qt 5.15
        add_library(Qt::Core INTERFACE IMPORTED)
        set_target_properties(Qt::Core PROPERTIES
            INTERFACE_LINK_LIBRARIES "Qt5::Core"
        )
        add_library(Qt::Widgets INTERFACE IMPORTED)
        set_target_properties(Qt::Widgets PROPERTIES
            INTERFACE_LINK_LIBRARIES "Qt5::Widgets"
        )
    endif()
endif()

# Based on https://stackoverflow.com/a/41199492 for easily deploying Qt DLLs on Windows.
if(WIN32 AND TARGET ${_qt_qmake_target} AND NOT TARGET Qt::windeployqt)
    get_target_property(_qt_qmake_location ${_qt_qmake_target} IMPORTED_LOCATION)

    execute_process(
        COMMAND "${_qt_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(EXISTS "${qt_install_prefix}/bin/windeployqt.exe")
        add_executable(Qt::windeployqt IMPORTED)

        set_target_properties(
            Qt::windeployqt PROPERTIES
            IMPORTED_LOCATION "${qt_install_prefix}/bin/windeployqt.exe"
        )
    endif()
endif()

# Allow opting into all this crazy Qt copying mess.
cmake_dependent_option(PLASMA_APPLOCAL_QT "Deploy Qt libraries into tool binary directories" OFF "TARGET Qt::windeployqt" OFF)
cmake_dependent_option(PLASMA_INSTALL_QT "Deploy Qt libraries when installing Plasma" ON "TARGET Qt::windeployqt" OFF)

function(plasma_deploy_qt)
    set(_DEPLOY_ARGS
        --no-translations
        --no-opengl-sw
    )
    if(Qt5_FOUND)
        list(APPEND _DEPLOY_ARGS --no-angle)
    endif()

    # Need to deploy Qt to the binary directory for each tool so it can be debugged more easily
    # in the IDE (this is analagous to VCPKG_APPLOCAL_DEPS). Note: add_custom_command cannot
    # attach to targets in a subdir.
    get_property(gui_tools GLOBAL PROPERTY _PLASMA_GUI_TOOLS)
    foreach(i IN LISTS gui_tools)
        if(PLASMA_APPLOCAL_QT)
            add_custom_target(
                ${i}_deployqt ALL
                COMMAND Qt::windeployqt ${_DEPLOY_ARGS} --dir "$<TARGET_FILE_DIR:${i}>" "$<TARGET_FILE:${i}>"
                WORKING_DIRECTORY "$<TARGET_FILE_DIR:Qt::windeployqt>"
            )
            add_dependencies(${i}_deployqt ${i})
        endif()
        list(APPEND _INSTALL_DEPLOY "$<TARGET_FILE:${i}>")
    endforeach()

    # Deploy only once on install
    if(PLASMA_INSTALL_QT AND _INSTALL_DEPLOY)
        string(JOIN [[" "]] _DEPLOY_ARG ${_INSTALL_DEPLOY})
        install(
            CODE
            "execute_process(
                COMMAND \"$<TARGET_FILE:Qt::windeployqt>\" ${_DEPLOY_ARGS} --dir \"${CMAKE_INSTALL_PREFIX}/tools\" \"${_DEPLOY_ARG}\"
                WORKING_DIRECTORY \"$<TARGET_FILE_DIR:Qt::windeployqt>\"
                COMMAND_ERROR_IS_FATAL ANY
            )"
        )
    endif()
endfunction()
