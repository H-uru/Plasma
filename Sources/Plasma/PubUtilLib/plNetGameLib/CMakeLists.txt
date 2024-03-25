set(plNetGameLib_PRIVATE
    Private/plNglAllIncludes.h
    Private/plNglAuth.h
    Private/plNglAuth.cpp
    Private/plNglCore.h
    Private/plNglCore.cpp
    Private/plNglFile.h
    Private/plNglFile.cpp
    Private/plNglGame.h
    Private/plNglGame.cpp
    Private/plNglGateKeeper.h
    Private/plNglGateKeeper.cpp
    Private/plNglMisc.cpp
    Private/plNglTrans.cpp
)

set(plNetGameLib_HEADERS
    Intern.h
    Pch.h
    plNetGameLib.h
)

# NOTE: Too many ODR violations and ambiguous names for UNITY_BUILD on this one.
plasma_library(plNetGameLib
    SOURCES ${plNetGameLib_PRIVATE} ${plNetGameLib_HEADERS}
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

source_group("Private" FILES ${plNetGameLib_PRIVATE})
source_group("Header Files" FILES ${plNetGameLib_HEADERS})
