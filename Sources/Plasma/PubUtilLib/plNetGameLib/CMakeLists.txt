include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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

source_group("Private" FILES ${plNetGameLib_PRIVATE})
source_group("Header Files" FILES ${plNetGameLib_HEADERS})
