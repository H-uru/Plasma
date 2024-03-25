set(pfAnimation_SOURCES
    pfObjectFlocker.cpp
    plAnimDebugList.cpp
    plBlower.cpp
    plFilterCoordInterface.cpp
    plFollowMod.cpp
    plLightModifier.cpp
    plLineFollowMod.cpp
    plRandomCommandMod.cpp
    plStereizer.cpp
    plViewFaceModifier.cpp
)

set(pfAnimation_HEADERS
    pfAnimationCreatable.h
    pfObjectFlocker.h
    plAnimDebugList.h
    plBlower.h
    plFilterCoordInterface.h
    plFollowMod.h
    plLightModifier.h
    plLineFollowMod.h
    plRandomCommandMod.h
    plStereizer.h
    plViewFaceModifier.h
)

plasma_library(pfAnimation SOURCES ${pfAnimation_SOURCES} ${pfAnimation_HEADERS})
target_link_libraries(
    pfAnimation
    PUBLIC
        CoreLib
        pnEncryption
        pnKeyedObject
        pnModifier
        pnSceneObject
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
        plAnimation
        plAvatar
        plGLight
        plInterp
        plMessage
        plPipeline
        plResMgr
        plSurface
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfAnimation_SOURCES})
source_group("Header Files" FILES ${pfAnimation_HEADERS})
