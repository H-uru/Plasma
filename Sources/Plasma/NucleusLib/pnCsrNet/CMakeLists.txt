include_directories(../../CoreLib)
include_directories(../../NucleusLib)

set(pnCsrNet_SOURCES
    pnCsrNet.cpp
)

set(pnCsrNet_HEADERS
    Pch.h
    pnCsrNet.h
)

add_library(pnCsrNet STATIC ${pnCsrNet_SOURCES} ${pnCsrNet_HEADERS})

source_group("Source Files" FILES ${pnCsrNet_SOURCES})
source_group("Header Files" FILES ${pnCsrNet_HEADERS})