set(plClientResMgr_SOURCES
    plClientResMgr.cpp
)

set(plClientResMgr_HEADERS
    plClientResMgr.h
)

add_library(plClientResMgr STATIC ${plClientResMgr_SOURCES} ${plClientResMgr_HEADERS})
target_link_libraries(
    plClientResMgr
    PUBLIC
        CoreLib
        string_theory
    PRIVATE
        pnNucleusInc
        plGImage
)

source_group("Source Files" FILES ${plClientResMgr_SOURCES})
source_group("Header Files" FILES ${plClientResMgr_HEADERS})
