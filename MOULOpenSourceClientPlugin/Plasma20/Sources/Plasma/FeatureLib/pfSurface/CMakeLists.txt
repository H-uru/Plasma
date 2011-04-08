include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

if(WIN32)
	add_definitions(-DWIN32)
endif(WIN32)

set(pfSurface_SOURCES
	plDistOpacityMod.cpp
	plFadeOpacityLay.cpp
	plFadeOpacityMod.cpp
	plGrabCubeMap.cpp
	plLayerAVI.cpp
	plLayerBink.cpp
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

add_library(pfSurface STATIC ${pfSurface_SOURCES} ${pfSources_HEADERS})

source_group("Source Files" FILES ${pfSurface_SOURCES})
source_group("Header Files" FILES ${pfSurface_HEADERS})
