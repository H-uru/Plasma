set(plProgressMgr_SOURCES
    plProgressMgr.cpp
)

set(plProgressMgr_HEADERS
    plProgressMgr.h
)

plasma_library(plProgressMgr
    FOLDER PubUtilLib
    SOURCES ${plProgressMgr_SOURCES} ${plProgressMgr_HEADERS}
)
target_link_libraries(
    plProgressMgr
    PUBLIC
        CoreLib
    PRIVATE
        plClientResMgr
        plPipeline
)

source_group("Source Files" FILES ${plProgressMgr_SOURCES})
source_group("Header Files" FILES ${plProgressMgr_HEADERS})
