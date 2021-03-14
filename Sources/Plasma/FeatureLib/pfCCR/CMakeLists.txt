set(pfCCR_SOURCES
    plCCRMgr.cpp
    plCCRVault.cpp
)

set(pfCCR_HEADERS
    plCCRCreatable.h
    plCCRMgr.h
    plCCRVault.h
)

plasma_library(pfCCR SOURCES ${pfCCR_SOURCES} ${pfCCR_HEADERS})
target_link_libraries(
    pfCCR
    PUBLIC
        CoreLib
        pnNucleusInc
)

source_group("Source Files" FILES ${pfCCR_SOURCES})
source_group("Header Files" FILES ${pfCCR_HEADERS})
