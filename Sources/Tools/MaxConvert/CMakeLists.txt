include_directories("../")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/FeatureLib")
include_directories("../../Plasma/FeatureLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/PubUtilLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories(${3dsm_INCLUDE_DIR})

set(MaxConvert_HEADERS
    hsControlConverter.h
    hsConverterUtils.h
    hsMaterialConverter.h
    hsMaxLayerBase.h
    hsVertexShader.h
    plBitmapCreator.h
    plClusterUtil.h
    plConvert.h
    plDistributor.h
    plDistTree.h
    plLayerConverter.h
    plLightMapGen.h
    plMaxLightContext.h
    plMeshConverter.h
    plRenderGlobalContext.h
    plRenderInstance.h
    StringTokenizer.h
    UserPropMgr.h
)

set(MaxConvert_SOURCES
    hsControlConverter.cpp
    hsConverterUtils.cpp
    hsMaterialConverter.cpp
    hsVertexShader.cpp
    plBitmapCreator.cpp
    plClusterUtil.cpp
    plConvert.cpp
    plDistributor.cpp
    plDistTree.cpp
    plLayerConverter.cpp
    plLightMapGen.cpp
    plMeshConverter.cpp
    plRenderGlobalContext.cpp
    plRenderInstance.cpp
    StringTokenizer.cpp
    UserPropMgr.cpp
)

use_precompiled_header(Pch.h Pch.cpp MaxConvert_HEADERS MaxConvert_SOURCES)
add_library(MaxConvert STATIC ${MaxConvert_HEADERS} ${MaxConvert_SOURCES})

source_group("Header Files" FILES ${MaxConvert_HEADERS})
source_group("Source Files" FILES ${MaxConvert_SOURCES})
