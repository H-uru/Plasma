set(pfGameScoreMgr_SOURCES
    pfGameScoreMgr.cpp
)

set(pfGameScoreMgr_HEADERS
    pfGameScoreMgr.h
)

add_library(pfGameScoreMgr STATIC ${pfGameScoreMgr_SOURCES} ${pfGameScoreMgr_HEADERS})
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
        string_theory
)

source_group("Source Files" FILES ${pfGameScoreMgr_SOURCES})
source_group("Header Files" FILES ${pfGameScoreMgr_HEADERS})
