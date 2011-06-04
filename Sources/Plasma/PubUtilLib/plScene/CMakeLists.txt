include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plScene_SOURCES
    plCullPoly.cpp
    plOccluder.cpp
    plOccluderProxy.cpp
    plPageTreeMgr.cpp
    plPostEffectMod.cpp
    plRelevanceMgr.cpp
    plRelevanceRegion.cpp
    plRenderRequest.cpp
    plSceneNode.cpp
    plVisMgr.cpp
    plVisRegion.cpp
)

set(plScene_HEADERS
    plCullPoly.h
    plOccluder.h
    plOccluderProxy.h
    plPageTreeMgr.h
    plPostEffectMod.h
    plRelevanceMgr.h
    plRelevanceRegion.h
    plRenderRequest.h
    plSceneCreatable.h
    plSceneNode.h
    plVisMgr.h
    plVisRegion.h
)

add_library(plScene STATIC ${plScene_SOURCES} ${plScene_HEADERS})

source_group("Source Files" FILES ${plScene_SOURCES})
source_group("Header Files" FILES ${plScene_HEADERS})
