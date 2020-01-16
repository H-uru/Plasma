include_directories(../../CoreLib)
include_directories(../../FeatureLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

include_directories(${OPENAL_INCLUDE_DIR})

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

add_library(pfCamera STATIC ${pfCamera_SOURCES} ${pfCamera_HEADERS})

source_group("Source Files" FILES ${pfCamera_SOURCES})
source_group("Header Files" FILES ${pfCamera_HEADERS})
