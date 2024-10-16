set(pfCharacter_SOURCES
    pfConfirmationMgr.cpp
    pfMarkerInfo.cpp
    pfMarkerMgr.cpp
)

set(pfCharacter_HEADERS
    pfCharacterCreatable.h
    pfConfirmationMgr.h
    pfMarkerInfo.h
    pfMarkerMgr.h
)

plasma_library(pfCharacter
    FOLDER FeatureLib
    SOURCES ${pfCharacter_SOURCES} ${pfCharacter_HEADERS}
)
target_link_libraries(
    pfCharacter
    PUBLIC
        CoreLib
        pnKeyedObject
        plMessage
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
        pnSceneObject
        plModifier
        plNetClient
        plResMgr
        plStatusLog
        pfGameGUIMgr
        pfLocalizationMgr
        pfMessage
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfCharacter_SOURCES})
source_group("Header Files" FILES ${pfCharacter_HEADERS})
