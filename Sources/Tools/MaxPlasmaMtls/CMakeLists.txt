include_directories("../")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/FeatureLib")
include_directories("../../Plasma/FeatureLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/PubUtilLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories(${3dsm_INCLUDE_DIR})

set(MaxPlasmaMtls_HEADERS
    plBMSampler.h
    plDetailCurveCtrl.h
    plMaterialRefMsg.h
    plMtlImport.h
    Shaders.h
)

set(MaxPlasmaMtls_HEADERS_Layers
    Layers/plAngleAttenLayer.h
    Layers/plDynamicEnvLayer.h
    Layers/plDynamicTextLayer.h
    Layers/plLayerTex.h
    Layers/plLayerTexBitmapPB.h
    Layers/plMAXCameraLayer.h
    Layers/plPlasmaMAXLayer.h
    Layers/plStaticEnvLayer.h
    Layers/plStaticEnvLayerBitmapPB.h
)

set(MaxPlasmaMtls_HEADERS_Materials
    Materials/plAnimStealthNode.h
    Materials/plBumpMtl.h
    Materials/plBumpMtlAnimPB.h
    Materials/plBumpMtlAnimPBDec.h
    Materials/plBumpMtlBasicPB.h
    Materials/plBumpMtlBasicPBDec.h
    Materials/plClothingMtl.h
    Materials/plClothingMtlPBDec.h
    Materials/plCompositeMtl.h
    Materials/plCompositeMtlDlg.h
    Materials/plCompositeMtlPB.h
    Materials/plCompositeMtlPBDec.h
    Materials/plDecalMtl.h
    Materials/plDecalMtlAdvPB.h
    Materials/plDecalMtlAdvPBDec.h
    Materials/plDecalMtlAnimPB.h
    Materials/plDecalMtlAnimPBDec.h
    Materials/plDecalMtlBasicPB.h
    Materials/plDecalMtlBasicPBDec.h
    Materials/plDecalMtlLayersPB.h
    Materials/plDecalMtlLayersPBDec.h
    Materials/plMultipassMtl.h
    Materials/plMultipassMtlDlg.h
    Materials/plMultipassMtlPB.h
    Materials/plNoteTrackWatcher.h
    Materials/plParticleMtl.h
    Materials/plParticleMtlPBDec.h
    Materials/plPassAnimDlgProc.h
    Materials/plPassBaseParamIDs.h
    Materials/plPassMtl.h
    Materials/plPassMtlAdvPB.h
    Materials/plPassMtlAdvPBDec.h
    Materials/plPassMtlAnimPB.h
    Materials/plPassMtlAnimPBDec.h
    Materials/plPassMtlBase.h
    Materials/plPassMtlBasicPB.h
    Materials/plPassMtlBasicPBDec.h
    Materials/plPassMtlLayersPB.h
    Materials/plPassMtlLayersPBDec.h
)

set(MaxPlasmaMtls_RESOURCES
    MaxPlasmaMtls.rc
    resource.h
    
    res/detailBgnd8bit.bmp
    res/dmtlbut.bmp
    res/dmtlmsk.bmp
)

set(MaxPlasmaMtls_SOURCES
    plBMSampler.cpp
    plDetailCurveCtrl.cpp
    plMtlImport.cpp
    Shaders.cpp
)

set(MaxPlasmaMtls_SOURCES_Layers
    Layers/plAngleAttenLayer.cpp
    Layers/plDynamicEnvLayer.cpp
    Layers/plDynamicTextLayer.cpp
    Layers/plLayerTex.cpp
    Layers/plMAXCameraLayer.cpp
    Layers/plPlasmaMAXLayer.cpp
    Layers/plStaticEnvLayer.cpp
)

set(MaxPlasmaMtls_SOURCES_Materials
    Materials/plAnimStealthConvert.cpp
    Materials/plAnimStealthNode.cpp
    Materials/plBumpMtl.cpp
    Materials/plClothingMtl.cpp
    Materials/plCompositeMtl.cpp
    Materials/plCompositeMtlDlg.cpp
    Materials/plDecalMtl.cpp
    Materials/plMultipassMtl.cpp
    Materials/plMultipassMtlDlg.cpp
    Materials/plNoteTrackWatcher.cpp
    Materials/plParticleMtl.cpp
    Materials/plPassAnimDlgProc.cpp
    Materials/plPassMtl.cpp
    Materials/plPassMtlBase.cpp
)

use_precompiled_header(Pch.h Pch.cpp MaxPlasmaMtls_HEADERS MaxPlasmaMtls_SOURCES)
add_library(MaxPlasmaMtls
            STATIC
            ${MaxPlasmaMtls_HEADERS}
            ${MaxPlasmaMtls_HEADERS_Layers}
            ${MaxPlasmaMtls_HEADERS_Materials}
            ${MaxPlasmaMtls_RESOURCES}
            ${MaxPlasmaMtls_SOURCES}
            ${MaxPlasmaMtls_SOURCES_Layers}
            ${MaxPlasmaMtls_SOURCES_Materials}
)

source_group("Header Files" FILES ${MaxPlasmaMtls_HEADERS})
source_group("Header Files\\Layers" FILES ${MaxPlasmaMtls_HEADERS_Layers})
source_group("Header Files\\Materials" FILES ${MaxPlasmaMtls_HEADERS_Materials})
source_group("Resource Files" FILES ${MaxPlasmaMtls_RESOURCES})
source_group("Source Files" FILES ${MaxPlasmaMtls_SOURCES})
source_group("Source Files\\Layers" FILES ${MaxPlasmaMtls_SOURCES_Layers})
source_group("Source Files\\Materials" FILES ${MaxPlasmaMtls_SOURCES_Materials})
