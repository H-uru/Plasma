set(plCrashHandler_SOURCES
    winmain.cpp
)
set(plCrashHandler_RESOURCES
    plCrashHandler.exe.manifest
    plCrashHandler.rc
    resource.h
)

plasma_executable(plCrashHandler WIN32 CLIENT SOURCES ${plCrashHandler_SOURCES} ${plCrashHandler_RESOURCES})
target_link_libraries(
    plCrashHandler
    PRIVATE
        CoreLib
        plClipboard
        plMessageBox
        plStatusLog
        plWinDpi
        pfCrashHandler

        $<$<PLATFORM_ID:Windows>:Comctl32>
        $<$<PLATFORM_ID:Windows>:UxTheme>
)
target_compile_definitions(
    plCrashHandler
    PRIVATE
        $<$<PLATFORM_ID:Windows>:UNICODE>
)

source_group("Source Files" FILES ${plCrashHandler_SOURCES})
source_group("Resource Files" FILES ${plCrashHandler_RESOURCES})

if(PLASMA_EXTERNAL_RELEASE)
    set_target_properties(plCrashHandler PROPERTIES OUTPUT_NAME "UruCrashHandler")
endif(PLASMA_EXTERNAL_RELEASE)
