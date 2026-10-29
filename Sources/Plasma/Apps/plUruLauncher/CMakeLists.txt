set(plUruLauncher_HEADERS
    plClientLauncher.h
)

set(plUruLauncher_SOURCES
    plClientLauncher.cpp
    winmain.cpp
)

set(plUruLauncher_RESOURCES
    plUruLauncher.exe.manifest
    plUruLauncher.rc
    resource.h

    banner.bmp
    Dirt.ICO
)

plasma_executable(plUruLauncher CLIENT
    FOLDER Apps
    SOURCES
        ${plUruLauncher_SOURCES} ${plUruLauncher_HEADERS}
        ${plUruLauncher_RESOURCES}
)
target_link_libraries(
    plUruLauncher
    PRIVATE
        CoreLib
        pnAsyncCore
        pnNetBase
        plMessageBox
        plNetGameLib
        plStatusLog
        plWinDpi
        pfConsoleCore
        pfPatcher
        CURL::libcurl
)

if(PLASMA_EXTERNAL_RELEASE)
    set_target_properties(plUruLauncher PROPERTIES OUTPUT_NAME UruLauncher)
endif(PLASMA_EXTERNAL_RELEASE)

source_group("Source Files" FILES ${plUruLauncher_SOURCES})
source_group("Header Files" FILES ${plUruLauncher_HEADERS})
source_group("Resource Files" FILES ${plUruLauncher_RESOURCES})
