include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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
target_link_libraries(plResMgr CoreLib)
target_link_libraries(plResMgr pnDispatch)
target_link_libraries(plResMgr pnFactory)
target_link_libraries(plResMgr pnKeyedObject)
target_link_libraries(plResMgr pnMessage)
target_link_libraries(plResMgr pnTimer)
target_link_libraries(plResMgr plAgeDescription)
target_link_libraries(plResMgr plFile)
target_link_libraries(plResMgr plStatusLog)

source_group("Source Files" FILES ${plResMgr_SOURCES})
source_group("Header Files" FILES ${plResMgr_HEADERS})
