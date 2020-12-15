include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plAudible_SOURCES
    plAudibleNull.cpp
    plWinAudible.cpp
    plWinAudibleProxy.cpp
)

set(plAudible_HEADERS
    plAudibleCreatable.h
    plAudibleNull.h
    plWinAudible.h
    plWinAudibleProxy.h
)

add_library(plAudible STATIC ${plAudible_SOURCES} ${plAudible_HEADERS})

source_group("Source Files" FILES ${plAudible_SOURCES})
source_group("Header Files" FILES ${plAudible_HEADERS})
