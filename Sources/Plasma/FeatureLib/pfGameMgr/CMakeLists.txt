set(pfGameMgr_HEADERS
    pfGameCli.h
    pfGameMgr.h
    pfGameMgrCreatable.h
    pfGameMgrTrans.h
    pfGmBlueSpiral.h
    pfGmMarker.h
)

set(pfGameMgr_SOURCES
    pfGameCli.cpp
    pfGameMgr.cpp
    pfGameMgrTrans.cpp
    pfGmBlueSpiral.cpp
    pfGmMarker.cpp
)

plasma_library(pfGameMgr
    FOLDER FeatureLib
    SOURCES ${pfGameMgr_HEADERS} ${pfGameMgr_SOURCES}
)
target_link_libraries(
    pfGameMgr
    PUBLIC
        CoreLib
        pnFactory
        pnGameMgr
    PRIVATE
        pnUtils
        pnUUID
        plNetClientComm
        plNetGameLib
)

source_group("Header Files" FILES ${pfGameMgr_HEADERS})
source_group("Source Files" FILES {pfGameMgr_SOURCES})
