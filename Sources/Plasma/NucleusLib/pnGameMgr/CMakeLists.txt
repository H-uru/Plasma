include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../NucleusLib/inc")

set(pnGameMgr_BLUESPIRAL
	BlueSpiral/pnGmBlueSpiral.h
	BlueSpiral/pnGmBlueSpiral.cpp
)

set(pnGameMgr_CLIMBINGWALL
	ClimbingWall/pnGmClimbingWall.h
	ClimbingWall/pnGmClimbingWall.cpp
)

set(pnGameMgr_HEEK
	Heek/pnGmHeek.h
	Heek/pnGmHeek.cpp
)

set(pnGameMgr_MARKER
	Marker/pnGmMarker.h
	Marker/pnGmMarker.cpp
)

set(pnGameMgr_TICTACTOE
	TicTacToe/pnGmTicTacToe.h
	TicTacToe/pnGmTicTacToe.cpp
)

set(pnGameMgr_VARSYNC
	VarSync/pnGmVarSync.h
	VarSync/pnGmVarSync.cpp
)

set(pnGameMgr_SOURCES
	Intern.h
	Pch.h
	pnGameMgr.h
	pnGameMgr.cpp
)

add_library(pnGameMgr STATIC ${pnGameMgr_SOURCES} ${pnGameMgr_BLUESPIRAL}
                             ${pnGameMgr_CLIMBINGWALL} ${pnGameMgr_HEEK}
                             ${pnGameMgr_MARKER} ${pnGameMgr_TICTACTOE}
                             ${pnGameMgr_VARSYNC})

source_group("BlueSpiral" FILES ${pnGameMgr_BLUESPIRAL})
source_group("ClimbingWall" FILES ${pnGameMgr_CLIMBINGWALL})
source_group("Heek" FILES ${pnGameMgr_HEEK})
source_group("Marker" FILES ${pnGameMgr_MARKER})
source_group("TicTacToe" FILES ${pnGameMgr_TICTACTOE})
source_group("VarSync" FILES ${pnGameMgr_VARSYNC})
source_group("" FILES ${pnGameMgr_SOURCES})
