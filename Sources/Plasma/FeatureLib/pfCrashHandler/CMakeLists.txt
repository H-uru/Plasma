set(pfCrashHandler_HEADERS
    plCrashCli.h
    plCrashBase.h
    plCrash_Private.h
    plCrashSrv.h
    plStackWalker.h
)

set(pfCrashHandler_SOURCES
    plCrashCli.cpp
    plCrashBase.cpp
    plCrashSrv.cpp
)

if(PLASMA_STACK_WALKER AND WIN32)
    list(APPEND pfCrashHandler_SOURCES
        plWindowsStackWalker.cpp
    )
else()
    # TODO: Implement for other platforms when that becomes relevant.
    list(APPEND pfCrashHandler_SOURCES
        plNullStackWalker.cpp
    )
endif()

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
