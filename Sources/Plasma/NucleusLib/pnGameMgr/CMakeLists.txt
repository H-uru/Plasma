set(pnGameMgr_HEADERS
    pnGameMgr.h
    pnGameMgrConst.h
    pnGmBlueSpiral.h
    pnGmClimbingWall.h
    pnGmClimbingWallConst.h
    pnGmHeek.h
    pnGmHeekConst.h
    pnGmMarker.h
    pnGmMarkerConst.h
    pnGmVarSync.h
)

plasma_library(pnGameMgr OBJECT
    FOLDER NucleusLib
    SOURCES ${pnGameMgr_HEADERS}
)
target_link_libraries(
    pnGameMgr
    INTERFACE
        CoreLib
        pnUUID
)

# This is needed because there are no .cpp files in the library.
set_target_properties(pnGameMgr PROPERTIES LINKER_LANGUAGE CXX)
