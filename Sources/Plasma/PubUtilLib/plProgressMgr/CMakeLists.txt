include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plProgressMgr_SOURCES
    plProgressMgr.cpp
)

set(plProgressMgr_HEADERS
    plProgressMgr.h
)

add_library(plProgressMgr STATIC ${plProgressMgr_SOURCES} ${plProgressMgr_HEADERS})

source_group("Source Files" FILES ${plProgressMgr_SOURCES})
source_group("Header Files" FILES ${plProgressMgr_HEADERS})
