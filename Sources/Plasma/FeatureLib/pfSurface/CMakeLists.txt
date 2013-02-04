include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

if(Bink_SDK_AVAILABLE)
    include_directories(${Bink_INCLUDE_DIR})
endif()

if(WIN32)
    include_directories(${DirectX_INCLUDE_DIR})
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
    plLayerBink.h
    plLayerMovie.h
)

add_library(pfSurface STATIC ${pfSurface_SOURCES} ${pfSurface_HEADERS})

source_group("Source Files" FILES ${pfSurface_SOURCES})
source_group("Header Files" FILES ${pfSurface_HEADERS})
