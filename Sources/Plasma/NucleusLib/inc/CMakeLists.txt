set(pnNucleusInc_HEADERS
    hsGDeviceRef.h
    hsGMatState.h
    hsGMatState.inl
    hsResMgr.h
    hsTimer.h
    plAudible.h
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
    plTimerCallbackManager.h
    pnAllCreatables.h
    pnNucleusCreatables.h
    pnTimerCreatable.h
)

set(pnNucleusInc_SOURCES
    plProfileManager.cpp
    pnSingletons.cpp
)

plasma_library(pnNucleusInc SOURCES ${pnNucleusInc_HEADERS} ${pnNucleusInc_SOURCES})
target_link_libraries(
    pnNucleusInc
    PUBLIC
        CoreLib

    # This target brings in "all of the nucleus" in theory and effectively in practice
    # with pnNucleusCreatables. So, we need to have almost everything as an interface.
    INTERFACE
        pnDispatch
        pnFactory
        pnKeyedObject
        pnMessage
        pnModifier
        pnNetCommon
        pnSceneObject
)

target_include_directories(pnNucleusInc INTERFACE ${CMAKE_CURRENT_LIST_DIR})

source_group("Header Files" FILES ${pnNucleusInc_HEADERS})
source_group("Source Files" FILES ${pnNucleusInc_SOURCES})
