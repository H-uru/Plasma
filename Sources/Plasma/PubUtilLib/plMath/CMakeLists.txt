include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plMath_SOURCES
    hsNoiseFunc.cpp
    hsRadixSort.cpp
    plAvg.cpp
    plTriUtils.cpp
)

set(plMath_HEADERS
    hsNoiseFunc.h
    hsRadixSort.h
    hsSearchVersion.h
    plAvg.h
    plTriUtils.h
)

add_library(plMath STATIC ${plMath_SOURCES} ${plMath_HEADERS})

source_group("Source Files" FILES ${plMath_SOURCES})
source_group("Header Files" FILES ${plMath_HEADERS})
