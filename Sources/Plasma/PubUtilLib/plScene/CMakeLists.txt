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

plasma_library(plScene
    SOURCES ${plScene_SOURCES} ${plScene_HEADERS}
    UNITY_BUILD
    PRECOMPILED_HEADER Pch.h
)
target_link_libraries(
    plScene
    PUBLIC
        CoreLib
        pnKeyedObject
        pnModifier
        pnSceneObject
        plDrawable
        plMessage
    PRIVATE
        pnMessage
        pnNucleusInc
        pnSceneObject
        plDrawable
        plGLight
        plIntersect
        plMath
        plPipeline
        plSurface
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plScene_SOURCES})
source_group("Header Files" FILES ${plScene_HEADERS})
