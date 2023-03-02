set(plPipeline_SOURCES
    hsG3DDeviceSelector.cpp
    pl3DPipeline.cpp
    plCaptureRender.cpp
    plCubicRenderTargetModifier.cpp
    plCullTree.cpp
    plDebugText.cpp
    plDTProgressMgr.cpp
    plDynamicEnvMap.cpp
    plFogEnvironment.cpp
    plPipelineViewSettings.cpp
    plPlates.cpp
    plRenderTarget.cpp
    plStatusLogDrawer.cpp
    plTextFont.cpp
    plTransitionMgr.cpp
)

set(plPipeline_HEADERS
    hsG3DDeviceSelector.h
    hsWinRef.h
    pl3DPipeline.h
    plCaptureRender.h
    plCubicRenderTarget.h
    plCubicRenderTargetModifier.h
    plCuller.h
    plCullTree.h
    plDebugText.h
    plDrawPrim.h
    plDTProgressMgr.h
    plDynamicEnvMap.h
    plFogEnvironment.h
    plNullPipeline.h
    plPipelineCreatable.h
    plPipelineViewSettings.h
    plPlates.h
    plRenderTarget.h
    plStatusLogDrawer.h
    plStencil.h
    plTextFont.h
    plTransitionMgr.h
)

plasma_library(plPipeline
    SOURCES ${plPipeline_SOURCES} ${plPipeline_HEADERS}
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(plPipeline
    PUBLIC
        CoreLib
        pnKeyedObject
        pnModifier
        pnNucleusInc
        plGImage
        plProgressMgr
        plScene
        plStatusLog
    PRIVATE
        pnMessage
        pnNetCommon
        pnSceneObject
        plAudio
        plAvatar
        plClientResMgr
        plDrawable
        plGLight
        plIntersect
        plMessage
        plNetClient
        plResMgr
        plSurface
        pfCamera # plCaptureRender, plDynamicEnvMap, plDXPipeline
        pfGameGUIMgr # plCaptureRender
    INTERFACE
        pnFactory
)

target_include_directories(plPipeline PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")

source_group("Source Files" FILES ${plPipeline_SOURCES})
source_group("Header Files" FILES ${plPipeline_HEADERS})
