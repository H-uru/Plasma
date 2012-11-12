include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../NucleusLib/inc")

set(pnGameMgr_GAMES
    BlueSpiral/pnGmBlueSpiral.h
    ClimbingWall/pnGmClimbingWall.h
    Heek/pnGmHeek.h
    Marker/pnGmMarker.h
    TicTacToe/pnGmTicTacToe.h
    VarSync/pnGmVarSync.h
)

set(pnGameMgr_SOURCES
    Intern.h
    Pch.h
    pnGameMgr.h

    # Needs at least one C++ file to show up in VS solution
    pnGameMgr.cpp
)

add_library(pnGameMgr STATIC ${pnGameMgr_SOURCES} ${pnGameMgr_GAMES})

source_group("Games" FILES ${pnGameMgr_GAMES})
source_group("" FILES ${pnGameMgr_SOURCES})
