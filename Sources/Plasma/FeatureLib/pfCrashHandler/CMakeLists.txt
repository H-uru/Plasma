set(pfCrashHandler_HEADERS
    plCrashCli.h
    plCrashBase.h
    plCrash_Private.h
    plCrashSrv.h
)

set(pfCrashHandler_SOURCES
    plCrashCli.cpp
    plCrashBase.cpp
    plCrashSrv.cpp
)

plasma_library(pfCrashHandler SOURCES ${pfCrashHandler_SOURCES} ${pfCrashHandler_HEADERS})
target_link_libraries(
    pfCrashHandler
    PUBLIC
        CoreLib
    PRIVATE
        $<$<PLATFORM_ID:Windows>:Dbghelp>
)

source_group("Source Files" FILES ${pfCrashHandler_SOURCES})
source_group("Header Files" FILES ${pfCrashHandler_HEADERS})
