include_directories(${PLASMA_SOURCE_ROOT}/CoreLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib/inc)
include_directories(${PLASMA_SOURCE_ROOT}/PubUtilLib)

if(WIN32)
    add_definitions(-DWIN32)
endif(WIN32)

set(pfSurface_SOURCES
    plDistOpacityMod.cpp
    plFadeOpacityLay.cpp
    plFadeOpacityMod.cpp
    plGrabCubeMap.cpp
    plLayerAVI.cpp
    plLayerMovie.cpp
)

set(pfSurface_HEADERS
    pfSurfaceCreatable.h
    plDistOpacityMod.h
    plFadeOpacityLay.h
    plFadeOpacityMod.h
    plGrabCubeMap.h
    plLayerAVI.h
    plLayerMovie.h
)

add_library(pfSurface STATIC ${pfSurface_SOURCES} ${pfSurface_HEADERS})

source_group("Source Files" FILES ${pfSurface_SOURCES})
source_group("Header Files" FILES ${pfSurface_HEADERS})
