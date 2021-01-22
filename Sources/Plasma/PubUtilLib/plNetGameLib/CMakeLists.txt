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

add_library(plNetGameLib STATIC ${plNetGameLib_PRIVATE} ${plNetGameLib_HEADERS})
target_link_libraries(
    plNetGameLib
    PUBLIC
        CoreLib
        pnEncryption
        pnNetBase
    PRIVATE
        pnAsyncCore
        pnNetBase
        pnNetCli
        pnNetCommon
        pnNetProtocol
        pnUtils
        plVault
)

# NOTE: Too many ODR violations and ambiguous names for UNITY_BUILD on this one.
plasma_optimize_target(plNetGameLib PRECOMPILED_HEADERS Pch.h)

source_group("Private" FILES ${plNetGameLib_PRIVATE})
source_group("Header Files" FILES ${plNetGameLib_HEADERS})
