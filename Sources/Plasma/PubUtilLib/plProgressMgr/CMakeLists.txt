set(plProgressMgr_SOURCES
    plProgressMgr.cpp
)

set(plProgressMgr_HEADERS
    plProgressMgr.h
)

add_library(plProgressMgr STATIC ${plProgressMgr_SOURCES} ${plProgressMgr_HEADERS})
target_link_libraries(
    plProgressMgr
    PUBLIC
        CoreLib
        string_theory
    PRIVATE
        plClientResMgr
        plPipeline
)

source_group("Source Files" FILES ${plProgressMgr_SOURCES})
source_group("Header Files" FILES ${plProgressMgr_HEADERS})
