include_directories(../../CoreLib)
include_directories(../../FeatureLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

set(pfGameMgr_SOURCES
    pfGameMgr.cpp
    BlueSpiral/pfGmBlueSpiral.cpp
    ClimbingWall/pfGmClimbingWall.cpp
    Heek/pfGmHeek.cpp
    Marker/pfGmMarker.cpp
    TicTacToe/pfGmTicTacToe.cpp
    VarSync/pfGmVarSync.cpp
)

set(pfGameMgr_HEADERS
    Intern.h
    Pch.h
    pfGameMgr.h
    pfGameMgrCreatables.h
    BlueSpiral/pfGmBlueSpiral.h
    ClimbingWall/pfGmClimbingWall.h
    Heek/pfGmHeek.h
    Marker/pfGmMarker.h
    TicTacToe/pfGmTicTacToe.h
    VarSync/pfGmVarSync.cpp
)

add_library(pfGameMgr ${pfGameMgr_SOURCES} ${pfGameMgr_HEADERS})

source_group("Source Files" FILES ${pfGameMgr_SOURCES})
source_group("Header Files" FILES ${pfGameMgr_HEADERS})
