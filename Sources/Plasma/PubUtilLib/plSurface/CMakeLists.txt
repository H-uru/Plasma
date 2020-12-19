set(plSurface_SOURCES
    hsGMaterial.cpp
    plGrassShaderMod.cpp
    plLayer.cpp
    plLayerAnimation.cpp
    plLayerDepth.cpp
    plLayerInterface.cpp
    plLayerMultiply.cpp
    plLayerOr.cpp
    plLayerShadowBase.cpp
    plShader.cpp
    plShaderTable.cpp
)

set(plSurface_HEADERS
    hsGMaterial.h
    plGrassShaderMod.h
    plLayer.h
    plLayerAnimation.h
    plLayerDepth.h
    plLayerInterface.h
    plLayerMultiply.h
    plLayerOr.h
    plLayerShadowBase.h
    plShader.h
    plShaderTable.h
    plSurfaceCreatable.h
)

set(plSurface_SHADERS
    ps_BiasNormals.h
    ps_CaddAAdd.h
    ps_CaddAbase.h
    ps_CaddAMult.h
    ps_CalphaAadd.h
    ps_CalphaAbase.h
    ps_CalphaAMult.h
    ps_CbaseAbase.h
    ps_CmultAAdd.h
    ps_CmultAbase.h
    ps_CmultAMult.h
    ps_CompCosines.h
    ps_GrassShader.h
    ps_MoreCosines.h
    ps_ShoreLeave6.h
    ps_WaveDecEnv.h
    ps_WaveFixed.h
    ps_WaveGraph.h
    ps_WaveGrid.h
    ps_WaveRip.h
    vs_BiasNormals.h
    vs_CompCosines.h
    vs_GrassShader.h
    vs_ShoreLeave6.h
    vs_ShoreLeave7.h
    vs_WaveDec1Lay.h
    vs_WaveDec1Lay_7.h
    vs_WaveDec2Lay11.h
    vs_WaveDec2Lay11_7.h
    vs_WaveDec2Lay12.h
    vs_WaveDec2Lay12_7.h
    vs_WaveDecEnv.h
    vs_WaveDecEnv_7.h
    vs_WaveFixedFin6.h
    vs_WaveFixedFin7.h
    vs_WaveGraph2.h
    vs_WaveGridFin.h
    vs_WaveRip.h
    vs_WaveRip7.h
)

add_library(plSurface STATIC ${plSurface_SOURCES} ${plSurface_HEADERS} ${plSurface_SHADERS})

target_link_libraries(plSurface
    PUBLIC
        CoreLib
        pnKeyedObject
        plGImage
        plInterp
        plMessage
)

source_group("Source Files" FILES ${plSurface_SOURCES})
source_group("Header Files" FILES ${plSurface_HEADERS})
source_group("Shaders" FILES ${plSurface_SHADERS})
