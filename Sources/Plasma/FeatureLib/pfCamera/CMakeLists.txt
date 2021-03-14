set(pfCamera_SOURCES
    pfCameraProxy.cpp
    plCameraBrain.cpp
    plCameraModifier.cpp
    plInterestingModifier.cpp
    plVirtualCamNeu.cpp
)

set(pfCamera_HEADERS
    pfCameraCreatable.h
    pfCameraProxy.h
    plCameraBrain.h
    plCameraModifier.h
    plInterestingModifier.h
    plVirtualCamNeu.h
)

plasma_library(pfCamera SOURCES ${pfCamera_SOURCES} ${pfCamera_HEADERS})
target_link_libraries(
    pfCamera
    PUBLIC
        CoreLib
        pnKeyedObject
        pnModifier
        plDrawable
    PRIVATE
        pnInputCore
        pnMessage
        pnNetCommon
        pnNucleusInc
        pnSceneObject
        plAvatar
        plInputCore
        plInterp
        plMessage
        plPipeline
        plScene
        plStatusLog
        pfAnimation
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfCamera_SOURCES})
source_group("Header Files" FILES ${pfCamera_HEADERS})
