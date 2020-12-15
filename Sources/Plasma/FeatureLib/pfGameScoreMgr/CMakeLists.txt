include_directories(${PLASMA_SOURCE_ROOT}/CoreLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib/inc)
include_directories(${PLASMA_SOURCE_ROOT}/PubUtilLib)
include_directories(${PLASMA_SOURCE_ROOT}/FeatureLib)

set(pfGameScoreMgr_SOURCES
    pfGameScoreMgr.cpp
)

set(pfGameScoreMgr_HEADERS
    pfGameScoreMgr.h
)

add_library(pfGameScoreMgr STATIC ${pfGameScoreMgr_SOURCES} ${pfGameScoreMgr_HEADERS})

source_group("Source Files" FILES ${pfGameScoreMgr_SOURCES})
source_group("Header Files" FILES ${pfGameScoreMgr_HEADERS})
