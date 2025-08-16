set(hsG3DDeviceDumper_SOURCES
    plAllCreatables.cpp
    main.cpp

    $<$<BOOL:${USE_WAYLAND}>:plWaylandDisplayHelper.cpp>
    $<$<BOOL:${USE_X11}>:plX11DisplayHelper.cpp>
)

set(hsG3DDeviceDumper_HEADERS
    $<$<BOOL:${USE_WAYLAND}>:plWaylandDisplayHelper.h>
    $<$<BOOL:${USE_WAYLAND}>:plWaylandFunctions.h>

    $<$<BOOL:${USE_X11}>:plX11DisplayHelper.h>
    $<$<BOOL:${USE_X11}>:plX11Functions.h>
)

plasma_executable(hsG3DDeviceDumper TOOL
    FOLDER Tools
    SOURCES
        ${hsG3DDeviceDumper_HEADERS} ${hsG3DDeviceDumper_SOURCES}

        # Hack hack hack
        $<$<PLATFORM_ID:Darwin>:../../Plasma/Apps/plClient/Mac-Cocoa/plMacDisplayHelper.mm>
        $<$<PLATFORM_ID:Windows>:../../Plasma/Apps/plClient/win32/plWinDisplayHelper.cpp>
)

target_include_directories(
    hsG3DDeviceDumper
    PRIVATE
    $<$<AND:$<BOOL:${USE_WAYLAND}>,$<TARGET_EXISTS:Wayland::Client>>:$<TARGET_PROPERTY:Wayland::Client,INCLUDE_DIRECTORIES>>
    $<$<AND:$<BOOL:${USE_X11}>,$<TARGET_EXISTS:X11::X11>>:$<TARGET_PROPERTY:X11::X11,INCLUDE_DIRECTORIES>>
    $<$<AND:$<BOOL:${USE_X11}>,$<TARGET_EXISTS:X11::Xrandr>>:$<TARGET_PROPERTY:X11::Xrandr,INCLUDE_DIRECTORIES>>
)

target_link_libraries(
    hsG3DDeviceDumper
    PRIVATE
        CoreLib

        # For the "all creatables"
        pnNucleusInc
        plPubUtilInc
        pfFeatureInc

        # Everything else used in this target.
        plPipeline
        $<$<BOOL:${PLASMA_PIPELINE_GL}>:pfGLPipeline>
        $<$<BOOL:${PLASMA_PIPELINE_METAL}>:pfMetalPipeline>
        "$<$<AND:$<PLATFORM_ID:Darwin>,$<VERSION_LESS:${CMAKE_OSX_DEPLOYMENT_TARGET},10.9>>:-framework ApplicationServices>"
        "$<$<AND:$<PLATFORM_ID:Darwin>,$<VERSION_GREATER_EQUAL:${CMAKE_OSX_DEPLOYMENT_TARGET},10.9>>:-framework CoreGraphics>"
)

source_group("Source Files" FILES ${hsG3DDeviceDumper_SOURCES})
source_group("Header Files" FILES ${hsG3DDeviceDumper_HEADERS})
