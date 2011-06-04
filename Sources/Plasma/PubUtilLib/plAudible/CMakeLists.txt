include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${OPENAL_INCLUDE_DIR})

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
