include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${OPENAL_INCLUDE_DIR})

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

add_library(plModifier STATIC ${plModifier_SOURCES} ${plModifier_HEADERS})

source_group("Source Files" FILES ${plModifier_SOURCES})
source_group("Header Files" FILES ${plModifier_HEADERS})
