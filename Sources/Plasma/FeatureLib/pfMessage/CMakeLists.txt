include_directories(${PLASMA_SOURCE_ROOT}/CoreLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib/inc)
include_directories(${PLASMA_SOURCE_ROOT}/PubUtilLib)
include_directories(${PLASMA_SOURCE_ROOT}/FeatureLib)

set(pfMessage_SOURCES
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
    plArmatureEffectMsg.h
    plClothingMsg.h
)

add_library(pfMessage STATIC ${pfMessage_SOURCES} ${pfMessage_HEADERS})

source_group("Source Files" FILES ${pfMessage_SOURCES})
source_group("Header Files" FILES ${pfMessage_HEADERS})
