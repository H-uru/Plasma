set(plModifier_SOURCES
    plAGMasterSDLModifier.cpp
    plAnimEventModifier.cpp
    plAnimTimeConvertSDLModifier.cpp
    plAxisAnimModifier.cpp
    plCloneSpawnModifier.cpp
    plDecalEnableMod.cpp
    plDetectorLog.cpp
    plExcludeRegionModifier.cpp
    plGameMarkerModifier.cpp
    plImageLibMod.cpp
    plInterfaceInfoModifier.cpp
    plLayerSDLModifier.cpp
    plLogicModifier.cpp
    plMaintainersMarkerModifier.cpp
    plResponderModifier.cpp
    plResponderSDLModifier.cpp
    plSDLModifier.cpp
    plSimpleModifier.cpp
    plSoundSDLModifier.cpp
    plSpawnModifier.cpp
)

set(plModifier_HEADERS
    plAGMasterSDLModifier.h
    plAnimEventModifier.h
    plAnimTimeConvertSDLModifier.h
    plAxisAnimModifier.h
    plCloneSpawnModifier.h
    plDecalEnableMod.h
    plDetectorLog.h
    plExcludeRegionModifier.h
    plGameMarkerModifier.h
    plImageLibMod.h
    plInterfaceInfoModifier.h
    plLayerSDLModifier.h
    plLogicModifier.h
    plMaintainersMarkerModifier.h
    plModifierCreatable.h
    plResponderModifier.h
    plResponderSDLModifier.h
    plSDLModifier.h
    plSimpleModifier.h
    plSoundSDLModifier.h
    plSpawnModifier.h
)

plasma_library(plModifier
    FOLDER PubUtilLib
    SOURCES ${plModifier_SOURCES} ${plModifier_HEADERS}
    UNITY_BUILD
    PRECOMPILED_HEADERS Pch.h
)

target_include_directories(plModifier PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")
target_link_libraries(plModifier
    PUBLIC
        CoreLib
        pnKeyedObject
        pnMessage
        pnModifier
        pnNetCommon
        plInterp # plSimpleModifier
        plStatusLog
    PRIVATE
        pnNucleusInc
        pnSceneObject
        plAnimation
        plAudio
        plAvatar
        plGImage
        plInputCore # plAxisAnimModifier, plLogicModifier
        plInterp
        plMessage
        plNetCommon
        plNetMessage
        plPhysical
        plPipeline # plResponderModifier
        plResMgr
        plSDL
        plSurface
        pfConditional # plLogicModifier
        pfMessage # plResponderModifier
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plModifier_SOURCES})
source_group("Header Files" FILES ${plModifier_HEADERS})
