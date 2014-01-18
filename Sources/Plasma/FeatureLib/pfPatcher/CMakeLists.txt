include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../NucleusLib/inc")
include_directories("../../PubUtilLib")

set(pfPatcher_SOURCES
    plManifests.cpp
    pfPatcher.cpp
)

set(pfPatcher_HEADERS
    plManifests.h
    pfPatcher.h
)

add_library(pfPatcher STATIC ${pfPatcher_SOURCES} ${pfPatcher_HEADERS})

source_group("Source Files" FILES ${pfPatcher_SOURCES})
source_group("Header Files" FILES ${pfPatcher_HEADERS})
