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
target_link_libraries(
    pfSurface
    PUBLIC
        CoreLib
        pnModifier
        plSurface
    PRIVATE
        pnNucleusInc
        pnSceneObject
        plAvatar
        plDrawable
        plGImage
        plMessage
        plScene
        $<$<PLATFORM_ID:Windows>:Vfw32>
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfSurface_SOURCES})
source_group("Header Files" FILES ${pfSurface_HEADERS})
