include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

set(pfGameScoreMgr_SOURCES
    pfGameScoreMgr.cpp
)

set(pfGameScoreMgr_HEADERS
    pfGameScoreMgr.h
)

add_library(pfGameScoreMgr STATIC ${pfGameScoreMgr_SOURCES} ${pfGameScoreMgr_HEADERS})

source_group("Source Files" FILES ${pfGameScoreMgr_SOURCES})
source_group("Header Files" FILES ${pfGameScoreMgr_HEADERS})
