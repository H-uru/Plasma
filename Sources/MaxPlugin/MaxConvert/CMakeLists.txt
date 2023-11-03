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
    UserPropMgr.cpp
)

plasma_library(MaxConvert
    SOURCES ${MaxConvert_HEADERS} ${MaxConvert_SOURCES}
    PRECOMPILED_HEADERS Pch.h
    NO_SANITIZE # The 3ds Max headers are full of clang diagnostic errors.
)
target_link_libraries(
    MaxConvert
    PUBLIC
        CoreLib
        pnEncryption
        pnNucleusInc
        plDrawable
        plInterp
    PRIVATE
        pnKeyedObject
        pnMessage
        pnSceneObject
        plAgeDescription
        plAvatar
        plGImage
        plGLight
        plMath
        plMessage
        plParticleSystem
        plPhysX
        plPipeline
        plResMgr
        plScene
        plSDL
        plStatusLog
        plSurface
        pfCamera
        pfSurface

        3dsm
)

source_group("Header Files" FILES ${MaxConvert_HEADERS})
source_group("Source Files" FILES ${MaxConvert_SOURCES})
