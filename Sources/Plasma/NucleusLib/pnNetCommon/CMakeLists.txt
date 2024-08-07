set(pnNetCommon_HEADERS
    plCreatableUuid.h
    plGenericVar.h
    plNetAddress.h
    plNetApp.h
    plNetGroup.h
    plNetSharedState.h
    plSDLTypes.h
    plSynchedObject.h
    pnNetCommon.h
    pnNetCommonCreatable.h
)

set(pnNetCommon_SOURCES
    plGenericVar.cpp
    plNetAddress.cpp
    plNetApp.cpp
    plNetGroup.cpp
    plNetSharedState.cpp
    plSDLTypes.cpp
    plSynchedObject.cpp
    pnNetCommon.cpp
)

plasma_library(pnNetCommon
    FOLDER NucleusLib
    SOURCES ${pnNetCommon_HEADERS} ${pnNetCommon_SOURCES}
)
target_link_libraries(
    pnNetCommon
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnNucleusInc
        pnUUID
    PRIVATE
        pnMessage
        pnSceneObject
        $<$<PLATFORM_ID:SunOS>:socket>
        $<$<PLATFORM_ID:Windows>:ws2_32>
)

source_group("Header Files" FILES ${pnNetCommon_HEADERS})
source_group("Source Files" FILES ${pnNetCommon_SOURCES})
