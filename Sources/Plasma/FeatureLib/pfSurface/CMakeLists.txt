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

plasma_library(pfSurface
    SOURCES ${pfSurface_SOURCES} ${pfSurface_HEADERS}
    UNITY_BUILD
    PRECOMPILED_HEADERS Pch.h
)
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
        $<$<PLATFORM_ID:Windows>:vfw32>
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfSurface_SOURCES})
source_group("Header Files" FILES ${pfSurface_HEADERS})
