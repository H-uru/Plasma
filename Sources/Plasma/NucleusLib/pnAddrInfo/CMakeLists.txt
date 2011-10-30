include_directories(../../CoreLib)

set(pnAddrInfo_SOURCES
    pnAddrInfo.cpp
)

set(pnAddrInfo_HEADERS
    pnAddrInfo.h
)

add_library(pnAddrInfo STATIC ${pnAddrInfo_SOURCES} ${pnAddrInfo_HEADERS})
target_link_libraries(pnAddrInfo CoreLib)
if(WIN32)
    target_link_libraries(pnAddrInfo ws2_32)
endif(WIN32)

source_group("Source Files" FILES ${pnAddrInfo_SOURCES})
source_group("Header Files" FILES ${pnAddrInfo_HEADERS})
