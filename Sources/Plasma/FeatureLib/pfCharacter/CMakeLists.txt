set(pfCharacter_SOURCES
    pfMarkerInfo.cpp
    pfMarkerMgr.cpp
)

set(pfCharacter_HEADERS
    pfCharacterCreatable.h
    pfMarkerInfo.h
    pfMarkerMgr.h
)

plasma_library(pfCharacter SOURCES ${pfCharacter_SOURCES} ${pfCharacter_HEADERS})
target_link_libraries(
    pfCharacter
    PUBLIC
        CoreLib
        pnKeyedObject
    PRIVATE
        pnMessage
        pnNucleusInc
        pnSceneObject
        plMessage
        plModifier
        plNetClient
        plResMgr
        plStatusLog
        pfMessage
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfCharacter_SOURCES})
source_group("Header Files" FILES ${pfCharacter_HEADERS})
