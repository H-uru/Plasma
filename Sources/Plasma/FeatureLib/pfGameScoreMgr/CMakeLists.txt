set(pfGameScoreMgr_SOURCES
    pfGameScoreMgr.cpp
)

set(pfGameScoreMgr_HEADERS
    pfGameScoreMgr.h
)

plasma_library(pfGameScoreMgr SOURCES ${pfGameScoreMgr_SOURCES} ${pfGameScoreMgr_HEADERS})
target_link_libraries(
    pfGameScoreMgr
    PUBLIC
        CoreLib
        pnKeyedObject
        pnNetBase
    PRIVATE
        pnNetProtocol
        plNetGameLib
        pfMessage
)

source_group("Source Files" FILES ${pfGameScoreMgr_SOURCES})
source_group("Header Files" FILES ${pfGameScoreMgr_HEADERS})
