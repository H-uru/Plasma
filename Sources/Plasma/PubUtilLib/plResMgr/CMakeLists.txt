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

add_library(plResMgr STATIC ${plResMgr_SOURCES} ${plResMgr_HEADERS})

target_link_libraries(plResMgr
    PUBLIC
        CoreLib
        pnDispatch
        pnFactory
        pnKeyedObject
        pnMessage
        pnTimer
        plAgeDescription
        plFile
        plStatusLog
)

source_group("Source Files" FILES ${plResMgr_SOURCES})
source_group("Header Files" FILES ${plResMgr_HEADERS})
