set(pfMessage_SOURCES
    pfBackdoorMsg.cpp
    pfGameGUIMsg.cpp
    pfGameScoreMsg.cpp
    pfGUINotifyMsg.cpp
    pfKIMsg.cpp
    pfMarkerMsg.cpp
    pfMovieEventMsg.cpp
    plArmatureEffectMsg.cpp
    plClothingMsg.cpp
)

set(pfMessage_HEADERS
    pfBackdoorMsg.h
    pfGameGUIMsg.h
    pfGameScoreMsg.h
    pfGUINotifyMsg.h
    pfKIMsg.h
    pfMarkerMsg.h
    pfMessageCreatable.h
    pfMovieEventMsg.h
    pfRemoteConsoleMsg.h
    plArmatureEffectMsg.h
    plClothingMsg.h
)

plasma_library(pfMessage SOURCES ${pfMessage_SOURCES} ${pfMessage_HEADERS})
target_link_libraries(
    pfMessage
    PUBLIC
        CoreLib
        pnMessage
        pnNetBase
    PRIVATE
        pnNucleusInc
        pfGameScoreMgr
)

source_group("Source Files" FILES ${pfMessage_SOURCES})
source_group("Header Files" FILES ${pfMessage_HEADERS})
