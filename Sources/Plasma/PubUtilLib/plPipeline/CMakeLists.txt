include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

if(PLASMA_PIPELINE STREQUAL "DirectX")
    include_directories(${DirectX_INCLUDE_DIR})
endif(PLASMA_PIPELINE STREQUAL "DirectX")

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
    hsFogControl.h
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

if(PLASMA_PIPELINE STREQUAL "DirectX")
    add_library(plPipeline STATIC ${plPipeline_SOURCES} ${plDXPipeline_SOURCES} ${plPipeline_HEADERS} ${plDXPipeline_HEADERS})

    target_link_libraries(plPipeline ${DirectX_LIBRARIES})
endif(PLASMA_PIPELINE STREQUAL "DirectX")

if(PLASMA_PIPELINE STREQUAL "OpenGL")
    add_library(plPipeline STATIC ${plPipeline_SOURCES} ${plGLPipeline_SOURCES} ${plPipeline_HEADERS} ${plGLPipeline_HEADERS})

    target_link_libraries(plPipeline EGL)
    target_link_libraries(plPipeline GLESv2)
endif(PLASMA_PIPELINE STREQUAL "OpenGL")

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
