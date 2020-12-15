include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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
