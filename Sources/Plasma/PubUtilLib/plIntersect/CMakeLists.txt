set(plIntersect_SOURCES
    plClosest.cpp
    plHardRegion.cpp
    plHardRegionPlanes.cpp
    plHardRegionTypes.cpp
    plSoftVolume.cpp
    plSoftVolumeTypes.cpp
    plVolumeIsect.cpp
)

set(plIntersect_HEADERS
    plClosest.h
    plHardRegion.h
    plHardRegionPlanes.h
    plHardRegionTypes.h
    plIntersectCreatable.h
    plRegionBase.h
    plSoftVolume.h
    plSoftVolumeTypes.h
    plVolumeIsect.h
)

plasma_library(plIntersect SOURCES ${plIntersect_SOURCES} ${plIntersect_HEADERS})

target_link_libraries(plIntersect
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnSceneObject
    PRIVATE
        pnNucleusInc
        pnMessage
        plMessage
)

source_group("Source Files" FILES ${plIntersect_SOURCES})
source_group("Header Files" FILES ${plIntersect_HEADERS})
