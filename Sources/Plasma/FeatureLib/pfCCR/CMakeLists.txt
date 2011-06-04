include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)

set(pfCCR_SOURCES
    plCCRMgr.cpp
    plCCRVault.cpp
)

set(pfCCR_HEADERS
    plCCRCreatable.h
    plCCRMgr.h
    plCCRVault.h
)

add_library(pfCCR STATIC ${pfCCR_SOURCES} ${pfCCR_HEADERS})

source_group("Source Files" FILES ${pfCCR_SOURCES})
source_group("Header Files" FILES ${pfCCR_HEADERS})
