include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

if(WIN32)
    include_directories(${DirectX_INCLUDE_DIR})
endif(WIN32)

set(plPipeline_SOURCES
    hsG3DDeviceSelector.cpp
    hsGColorizer.cpp
    plCaptureRender.cpp
    plCubicRenderTargetModifier.cpp
    plCullTree.cpp
    plDebugText.cpp
    plDTProgressMgr.cpp
    plDynamicEnvMap.cpp
    plFogEnvironment.cpp
    plGBufferGroup.cpp
    plPlates.cpp
    plRenderTarget.cpp
    plStatusLogDrawer.cpp
    plTextFont.cpp
    plTextGenerator.cpp
    plTransitionMgr.cpp
    plVertCoder.cpp
)

set(plDXPipeline_SOURCES
    DX/hsGDirect3D.cpp
    DX/plDXDeviceRefs.cpp
    DX/plDXEnumerate.cpp
    DX/plDXPipeline.cpp
    DX/plDXPixelShader.cpp
    DX/plDXShader.cpp
    DX/plDXTextFont.cpp
    DX/plDXVertexShader.cpp
)

set(plPipeline_HEADERS
    hsFogControl.h
    hsG3DDeviceSelector.h
    hsGColorizer.h
    hsGDeviceRef.h
    hsWinRef.h
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
    plGBufferGroup.h
    plPipeDebugFlags.h
    plPipelineCreatable.h
    plPipelineCreate.h
    plPlates.h
    plRenderTarget.h
    plStatusLogDrawer.h
    plStencil.h
    plTextFont.h
    plTextGenerator.h
    plTransitionMgr.h
    plVertCoder.h
)

set(plDXPipeline_HEADERS
    DX/hsGDirect3D.h
    DX/plDXBufferRefs.h
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

if(WIN32)
    add_library(plPipeline STATIC ${plPipeline_SOURCES} ${plDXPipeline_SOURCES} ${plPipeline_HEADERS} ${plDXPipeline_HEADERS})
    target_link_libraries(plPipeline ${DirectX_LIBRARIES})
else()
    add_library(plPipeline STATIC ${plPipeline_SOURCES} ${plPipeline_HEADERS})
endif()

source_group("Source Files" FILES ${plPipeline_SOURCES})
source_group("Header Files" FILES ${plPipeline_HEADERS})

if(WIN32)
    source_group("DX\\Source Files" FILES ${plDXPipeline_SOURCES})
    source_group("DX\\Header Files" FILES ${plDXPipeline_HEADERS})
endif()
