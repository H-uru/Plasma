include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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
