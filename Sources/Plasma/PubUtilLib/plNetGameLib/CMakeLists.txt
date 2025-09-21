set(plNetGameLib_HEADERS
    Intern.h
    Pch.h
    plNetGameLib.h
    plNglAuth.h
    plNglCore.h
    plNglFile.h
    plNglGame.h
    plNglGateKeeper.h
)

set(plNetGameLib_SOURCES
    plNglAuth.cpp
    plNglCore.cpp
    plNglFile.cpp
    plNglGame.cpp
    plNglGateKeeper.cpp
    plNglMisc.cpp
    plNglTrans.cpp
)

# NOTE: Too many ODR violations and ambiguous names for UNITY_BUILD on this one.
plasma_library(plNetGameLib
    FOLDER PubUtilLib
    SOURCES ${plNetGameLib_HEADERS} ${plNetGameLib_SOURCES}
    PRECOMPILED_HEADERS Pch.h
)

target_link_libraries(
    plNetGameLib
    PUBLIC
        CoreLib
        pnEncryption
        pnNetBase
    PRIVATE
        pnAsyncCore
        pnGameMgr
        pnNetBase
        pnNetCli
        pnNetCommon
        pnNetProtocol
        pnUtils
        plVault
)

source_group("Header Files" FILES ${plNetGameLib_HEADERS})
source_group("Source Files" FILES ${plNetGameLib_SOURCES})
