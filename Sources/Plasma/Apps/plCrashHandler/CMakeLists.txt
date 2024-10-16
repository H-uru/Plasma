set(plCrashHandler_SOURCES
    winmain.cpp
)
set(plCrashHandler_RESOURCES
    plCrashHandler.exe.manifest
    plCrashHandler.rc
    resource.h
)

plasma_executable(plCrashHandler CLIENT
    FOLDER Apps
    SOURCES ${plCrashHandler_SOURCES} ${plCrashHandler_RESOURCES}
)
target_link_libraries(
    plCrashHandler
    PRIVATE
        CoreLib
        plClipboard
        plMessageBox
        plStatusLog
        plWinDpi
        pfCrashHandler

        $<$<PLATFORM_ID:Windows>:comctl32>
        $<$<PLATFORM_ID:Windows>:uxtheme>
)
target_compile_definitions(
    plCrashHandler
    PRIVATE
        $<$<PLATFORM_ID:Windows>:UNICODE>
)

if(MINGW)
    target_link_options(plCrashHandler PRIVATE -municode)
endif()

source_group("Source Files" FILES ${plCrashHandler_SOURCES})
source_group("Resource Files" FILES ${plCrashHandler_RESOURCES})

if(PLASMA_EXTERNAL_RELEASE)
    set_target_properties(plCrashHandler PROPERTIES OUTPUT_NAME "UruCrashHandler")
endif(PLASMA_EXTERNAL_RELEASE)
