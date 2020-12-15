include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plStatGather_SOURCES
    plAutoProfile.cpp
    plCalculatedProfiles.cpp
    plProfileManagerFull.cpp
)

set(plStatGather_HEADERS
    plAutoProfile.h
    plCalculatedProfiles.h
    plProfileManagerFull.h
    plStatGatherCreatable.h
)

add_library(plStatGather STATIC ${plStatGather_SOURCES} ${plStatGather_HEADERS})

source_group("Source Files" FILES ${plStatGather_SOURCES})
source_group("Header Files" FILES ${plStatGather_HEADERS})
