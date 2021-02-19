find_package(Qt5 COMPONENTS Core Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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
        set(CAN_DEPLOY_QT ON)
    endif()
endif()

# Allow opting into all this crazy Qt copying mess.
cmake_dependent_option(PLASMA_APPLOCAL_QT "Deploy Qt libraries into tool binary directories" OFF CAN_DEPLOY_QT OFF)
cmake_dependent_option(PLASMA_INSTALL_QT "Deploy Qt libraries when installing Plasma" ON CAN_DEPLOY_QT OFF)

function(_add_tools)
    cmake_parse_arguments(_at "GUI" "" "TOOLS" ${ARGN})
    if(NOT _at_TOOLS)
        message(FATAL_ERROR "_add_tools() called with no TOOLS, you tool.")
    endif()

    foreach(i IN LISTS _at_TOOLS)
        add_subdirectory(${i})
        if(NOT TARGET ${i})
            message(FATAL_ERROR "_add_tools() requires that tool directory and target name match!")
        endif()
    endforeach()

    add_dependencies(tools ${_at_TOOLS})
    if(_at_GUI)
        install(TARGETS ${_at_TOOLS} DESTINATION tools_gui)
        set(_TOOLS_GUI ${_at_TOOLS} ${_TOOLS_GUI} PARENT_SCOPE)
    else()
        install(TARGETS ${_at_TOOLS} DESTINATION tools_cli)
    endif()
endfunction()

function(_deploy_qt)
    # Need to deploy Qt to the binary directory for each tool so it can be debugged more easily
    # in the IDE (this is analagous to VCPKG_APPLOCAL_DEPS). Note: add_custom_command cannot
    # attach to targets in a subdir.
    foreach(i IN LISTS _TOOLS_GUI)
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
                COMMAND \"$<TARGET_FILE:Qt5::windeployqt>\" --no-translations --no-angle --no-opengl-sw --dir \"${CMAKE_INSTALL_PREFIX}/tools_gui\" \"${_DEPLOY_ARG}\"
                WORKING_DIRECTORY \"$<TARGET_FILE_DIR:Qt5::windeployqt>\"
                COMMAND_ERROR_IS_FATAL ANY
            )"
        )
    endif()
endfunction()

include_directories("${PLASMA_TOOLS_ROOT}")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

if(PLASMA_BUILD_TOOLS)
    _add_tools(TOOLS SoundDecompress)

    if(Qt5_FOUND)
        _add_tools(GUI TOOLS plLocalizationEditor plResBrowser)
    endif()

    if(WIN32)
        _add_tools(TOOLS plShaderAssembler)
    endif()

    if(TARGET freetype AND Qt5_FOUND)
        _add_tools(GUI TOOLS plFontConverter)
    endif()

    _deploy_qt()
endif()

add_subdirectory(plLocalizationBenchmark)

# Max Stuff goes below here...
if(3dsm_FOUND AND 3dsm_BUILD_PLUGIN)
    add_subdirectory(MaxComponent)
    add_subdirectory(MaxConvert)
    add_subdirectory(MaxExport)
    add_subdirectory(MaxMain)
    add_subdirectory(MaxPlasmaLights)
    add_subdirectory(MaxPlasmaMtls)
endif()
