include_directories(${PLASMA_SOURCE_ROOT}/CoreLib)
include_directories(${PLASMA_SOURCE_ROOT}/FeatureLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib/inc)
include_directories(${PLASMA_SOURCE_ROOT}/PubUtilLib)

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
