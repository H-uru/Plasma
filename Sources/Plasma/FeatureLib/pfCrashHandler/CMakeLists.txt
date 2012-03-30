include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../PubUtilLib)

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

add_library(pfCrashHandler STATIC ${pfCrashHandler_SOURCES} ${pfCrashHandler_HEADERS})

source_group("Source Files" FILES ${pfCrashHandler_SOURCES})
source_group("Header Files" FILES ${pfCrashHandler_HEADERS})
