include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../NucleusLib/inc")

set(pnNucleusInc_HEADERS
    hsGDeviceRef.h
    hsGMatState.h
    hsGMatState.inl
    hsResMgr.h
    hsTimer.h
    plAudible.h
    plCCRMgrBase.h
    plClassIndexMacros.h
    plCreatableIndex.h
    plCreatableStrings.h
    plDrawable.h
    plgDispatch.h
    plLoggable.h
    plPhysical.h
    plPipeDebugFlags.h
    plPipeline.h
    plPipeResReq.h
    plProfile.h
    plProfileManager.h
    plRefFlags.h
    pnAllCreatables.h
    pnNucleusCreatables.h
)

set(pnNucleusInc_SOURCES
    plProfileManager.cpp
    pnSingletons.cpp
)

add_library(pnNucleusInc STATIC ${pnNucleusInc_HEADERS} ${pnNucleusInc_SOURCES})
target_link_libraries(pnNucleusInc CoreLib pnTimer)

source_group("Header Files" FILES ${pnNucleusInc_HEADERS})
source_group("Source Files" FILES ${pnNucleusInc_SOURCES})
