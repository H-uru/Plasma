include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plProgressMgr_SOURCES
    plProgressMgr.cpp
)

set(plProgressMgr_HEADERS
    plProgressMgr.h
)

add_library(plProgressMgr STATIC ${plProgressMgr_SOURCES} ${plProgressMgr_HEADERS})

source_group("Source Files" FILES ${plProgressMgr_SOURCES})
source_group("Header Files" FILES ${plProgressMgr_HEADERS})
