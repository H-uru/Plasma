include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plMath_SOURCES
    hsRadixSort.cpp
    plTriUtils.cpp
)

set(plMath_HEADERS
    hsRadixSort.h
    plTriUtils.h
)

add_library(plMath STATIC ${plMath_SOURCES} ${plMath_HEADERS})

source_group("Source Files" FILES ${plMath_SOURCES})
source_group("Header Files" FILES ${plMath_HEADERS})
