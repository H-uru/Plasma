set(plClientResMgr_SOURCES
    plClientResMgr.cpp
)

set(plClientResMgr_HEADERS
    plClientResMgr.h
)

plasma_library(plClientResMgr SOURCES ${plClientResMgr_SOURCES} ${plClientResMgr_HEADERS})
target_link_libraries(
    plClientResMgr
    PUBLIC
        CoreLib
    PRIVATE
        pnNucleusInc
        plGImage
)

source_group("Source Files" FILES ${plClientResMgr_SOURCES})
source_group("Header Files" FILES ${plClientResMgr_HEADERS})
