set(pfDisplayHelpers_SOURCES
    $<$<PLATFORM_ID:Darwin>:plMacDisplayHelper.mm>
    $<$<PLATFORM_ID:Windows>:plWinDisplayHelper.cpp>
    $<$<BOOL:${USE_WAYLAND}>:plWaylandDisplayHelper.cpp>
    $<$<BOOL:${USE_X11}>:plX11DisplayHelper.cpp>
)

set(pfDisplayHelpers_HEADERS
    $<$<PLATFORM_ID:Darwin>:plMacDisplayHelper.h>
    $<$<PLATFORM_ID:Windows>:plWinDisplayHelper.h>
    $<$<BOOL:${USE_WAYLAND}>:plWaylandDisplayHelper.h>
    $<$<BOOL:${USE_X11}>:plX11DisplayHelper.h>

    $<$<BOOL:${USE_WAYLAND}>:plWaylandFunctions.h>
    $<$<BOOL:${USE_X11}>:plX11Functions.h>
)

plasma_library(pfDisplayHelpers
    FOLDER FeatureLib
    SOURCES ${pfDisplayHelpers_SOURCES} ${pfDisplayHelpers_HEADERS}
)
target_link_libraries(
    pfDisplayHelpers
    PUBLIC
        CoreLib
        plPipeline
        "$<$<AND:$<PLATFORM_ID:Darwin>,$<VERSION_LESS:${CMAKE_OSX_DEPLOYMENT_TARGET},10.9>>:-framework ApplicationServices>"
        "$<$<AND:$<PLATFORM_ID:Darwin>,$<VERSION_GREATER_EQUAL:${CMAKE_OSX_DEPLOYMENT_TARGET},10.9>>:-framework CoreGraphics>"
)

# We're not linking against X11 or Wayland because we want to load them
# dynamically at runtime, but we do need their include paths to be added here
# so the header definitions can be loaded
target_include_directories(
    pfDisplayHelpers
    PUBLIC
        $<$<BOOL:${USE_WAYLAND}>:$<TARGET_PROPERTY:Wayland::Client,INCLUDE_DIRECTORIES>>
        $<$<BOOL:${USE_X11}>:$<TARGET_PROPERTY:X11::X11,INCLUDE_DIRECTORIES>>
        $<$<BOOL:${USE_XRANDR}>:$<TARGET_PROPERTY:X11::Xrandr,INCLUDE_DIRECTORIES>>
)

source_group("Source Files" FILES ${pfDisplayHelpers_SOURCES})
source_group("Header Files" FILES ${pfDisplayHelpers_HEADERS})
