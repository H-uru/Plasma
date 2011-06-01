include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plClientResMgr_SOURCES
    plClientResMgr.cpp
)

set(plClientResMgr_HEADERS
    plClientResMgr.h
)

add_library(plClientResMgr STATIC ${plClientResMgr_SOURCES} ${plClientResMgr_HEADERS})

source_group("Source Files" FILES ${plClientResMgr_SOURCES})
source_group("Header Files" FILES ${plClientResMgr_HEADERS})
