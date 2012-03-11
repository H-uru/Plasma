include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnSimpleNet_HEADERS
    pnSimpleNet.h
)

set(pnSimpleNet_SOURCES
    pnSimpleNet.cpp
)

add_library(pnSimpleNet STATIC ${pnSimpleNet_HEADERS} ${pnSimpleNet_SOURCES})

source_group("Header Files" FILES ${pnSimpleNet_HEADERS})
source_group("Source Files" FILES ${pnSimpleNet_SOURCES})
