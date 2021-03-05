set(plResMgr_SOURCES
    plKeyFinder.cpp
    plLocalization.cpp
    plPageInfo.cpp
    plRegistryHelpers.cpp
    plRegistryKeyList.cpp
    plRegistryNode.cpp
    plResManager.cpp
    plResManagerHelper.cpp
    plVersion.cpp
)

set(plResMgr_HEADERS
    plKeyFinder.h
    plLocalization.h
    plPageInfo.h
    plRegistryHelpers.h
    plRegistryKeyList.h
    plRegistryNode.h
    plResManager.h
    plResManagerHelper.h
    plResMgrCreatable.h
    plResMgrSettings.h
    plVersion.h
)

plasma_library(plResMgr
    SOURCES ${plResMgr_SOURCES} ${plResMgr_HEADERS}
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(
    plResMgr
    PUBLIC
        CoreLib
        pnKeyedObject
        pnNucleusInc
    PRIVATE
        pnDispatch
        pnMessage
        pnNetCommon
        plMessage
        plStatusLog
    INTERFACE
        pnFactory
)

# Good luck with this...
option(PLASMA_RESMGR_DEBUGGING "Use mcn's ResManager debug console" OFF)
target_compile_definitions(
    plResMgr
    PUBLIC # This is used in headers and sources, sadly...
        $<$<BOOL:${PLASMA_RESMGR_DEBUGGING}>:MCN_RESMGR_DEBUGGING>
)
target_link_libraries(
    plResMgr
    PRIVATE
        $<$<BOOL:${PLASMA_RESMGR_DEBUGGING}>:pnInputCore>
        $<$<BOOL:${PLASMA_RESMGR_DEBUGGING}>:plInputCore>
)

source_group("Source Files" FILES ${plResMgr_SOURCES})
source_group("Header Files" FILES ${plResMgr_HEADERS})
