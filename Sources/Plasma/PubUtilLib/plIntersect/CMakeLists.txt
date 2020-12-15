include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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

add_library(plIntersect STATIC ${plIntersect_SOURCES} ${plIntersect_HEADERS})

source_group("Source Files" FILES ${plIntersect_SOURCES})
source_group("Header Files" FILES ${plIntersect_HEADERS})
