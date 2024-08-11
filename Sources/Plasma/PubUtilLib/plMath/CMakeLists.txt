set(plMath_SOURCES
    hsRadixSort.cpp
    plTriUtils.cpp
)

set(plMath_HEADERS
    hsRadixSort.h
    plTriUtils.h
)

plasma_library(plMath
    FOLDER PubUtilLib
    SOURCES ${plMath_SOURCES} ${plMath_HEADERS}
)
target_link_libraries(
    plMath
    PUBLIC
        CoreLib
)

source_group("Source Files" FILES ${plMath_SOURCES})
source_group("Header Files" FILES ${plMath_HEADERS})
