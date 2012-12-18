include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

if(Bink_SDK_AVAILABLE)
    include_directories(${Bink_INCLUDE_DIR})
endif()
if(WIN32)
    include_directories(${DirectX_INCLUDE_DIR})
endif(WIN32)

set(plPipeline_SOURCES
    hsG3DDeviceSelector.cpp
    hsGColorizer.cpp
    hsGDDrawDllLoad.cpp
    plCaptureRender.cpp
    plCubicRenderTargetModifier.cpp
    plCullTree.cpp
    plDebugText.cpp
    plDTProgressMgr.cpp
    plDXEnumerate.cpp
    plDXPipeline.cpp
    plDXPixelShader.cpp
    plDXShader.cpp
    plDXTextFont.cpp
    plDXVertexShader.cpp
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

set(plPipeline_HEADERS
    hsFogControl.h
    hsG3DDeviceSelector.h
    hsGColorizer.h
    hsGDDrawDllLoad.h
    hsWinRef.h
    plBinkPlayer.h
    plCaptureRender.h
    plCubicRenderTarget.h
    plCubicRenderTargetModifier.h
    plCuller.h
    plCullTree.h
    plDebugText.h
    plDrawPrim.h
    plDTProgressMgr.h
    plDXEnumerate.h
    plDXPipeline.h
    plDXPixelShader.h
    plDXSettings.h
    plDXShader.h
    plDXTextFont.h
    plDXVertexShader.h
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

set(plPipeline_DEVICEREFS
    hsGDeviceRef.h
    plDXBufferRefs.h
    plDXDeviceRef.h
    plDXDeviceRefs.cpp
    plDXLightRef.h
    plDXRenderTargetRef.h
    plDXTextureRef.h
)

add_library(plPipeline STATIC ${plPipeline_SOURCES} ${plPipeline_HEADERS} ${plPipeline_DEVICEREFS})

source_group("Source Files" FILES ${plPipeline_SOURCES})
source_group("Header Files" FILES ${plPipeline_HEADERS})
source_group("DeviceRefs" FILES ${plPipeline_DEVICEREFS})
