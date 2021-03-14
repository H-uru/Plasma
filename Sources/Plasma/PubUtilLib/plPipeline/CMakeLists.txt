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
    plTextGenerator.cpp
    plTransitionMgr.cpp
)

set(plDXPipeline_SOURCES
    DX/hsGDirect3D.cpp
    DX/plDXDevice.cpp
    DX/plDXDeviceRefs.cpp
    DX/plDXEnumerate.cpp
    DX/plDXPipeline.cpp
    DX/plDXPixelShader.cpp
    DX/plDXShader.cpp
    DX/plDXTextFont.cpp
    DX/plDXVertexShader.cpp
)

set(plGLPipeline_SOURCES
    GL/plGLDevice.cpp
    GL/plGLDeviceRefs.cpp
    GL/plGLPipeline.cpp
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
    plPipelineCreatable.h
    plPipelineCreate.h
    plPipelineViewSettings.h
    plPlates.h
    plRenderTarget.h
    plStatusLogDrawer.h
    plStencil.h
    plTextFont.h
    plTextGenerator.h
    plTransitionMgr.h
)

set(plDXPipeline_HEADERS
    DX/hsGDirect3D.h
    DX/plDXBufferRefs.h
    DX/plDXDevice.h
    DX/plDXDeviceRef.h
    DX/plDXEnumerate.h
    DX/plDXLightRef.h
    DX/plDXPipeline.h
    DX/plDXPixelShader.h
    DX/plDXRenderTargetRef.h
    DX/plDXSettings.h
    DX/plDXShader.h
    DX/plDXTextFont.h
    DX/plDXTextureRef.h
    DX/plDXVertexShader.h
)

set(plGLPipeline_HEADERS
    GL/plGLDevice.h
    GL/plGLDeviceRef.h
    GL/plGLPipeline.h
)

set(plPipeline_ALL_FILES
    ${plPipeline_SOURCES}
    ${plPipeline_HEADERS}
)

set(plPipeline_PCH
    Pch.h
)

if(PLASMA_PIPELINE STREQUAL "DirectX")
    list(APPEND plPipeline_ALL_FILES ${plDXPipeline_SOURCES} ${plDXPipeline_HEADERS})
    list(APPEND plPipeline_PCH DX/Pch.h)
endif(PLASMA_PIPELINE STREQUAL "DirectX")

if(PLASMA_PIPELINE STREQUAL "OpenGL")
    find_package(OpenGL REQUIRED)
    list(APPEND plPipeline_ALL_FILES ${plGLPipeline_SOURCES} ${plGLPipeline_HEADERS})
endif(PLASMA_PIPELINE STREQUAL "OpenGL")

plasma_library(plPipeline
    SOURCES ${plPipeline_ALL_FILES}
    PRECOMPILED_HEADERS ${plPipeline_PCH}
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
        $<$<STREQUAL:${PLASMA_PIPELINE},DirectX>:${DirectX_LIBRARIES}>
        $<$<STREQUAL:${PLASMA_PIPELINE},OpenGL>:OpenGL::GL>
    INTERFACE
        pnFactory
)

target_include_directories(plPipeline PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")

source_group("Source Files" FILES ${plPipeline_SOURCES})
source_group("Header Files" FILES ${plPipeline_HEADERS})

if(PLASMA_PIPELINE STREQUAL "DirectX")
    source_group("DX\\Source Files" FILES ${plDXPipeline_SOURCES})
    source_group("DX\\Header Files" FILES ${plDXPipeline_HEADERS})
endif(PLASMA_PIPELINE STREQUAL "DirectX")

if(PLASMA_PIPELINE STREQUAL "OpenGL")
    source_group("GL\\Source Files" FILES ${plGLPipeline_SOURCES})
    source_group("GL\\Header Files" FILES ${plGLPipeline_HEADERS})
endif(PLASMA_PIPELINE STREQUAL "OpenGL")
