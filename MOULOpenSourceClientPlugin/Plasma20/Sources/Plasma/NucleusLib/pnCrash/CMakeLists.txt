include_directories(../../CoreLib)
include_directories(../../NucleusLib)

set(pnCrash_SOURCES
    pnCrash.cpp
)

set(pnCrash_HEADERS
    Pch.h
    pnCrash.h
)

add_library(pnCrash STATIC ${pnCrash_SOURCES} ${pnCrash_HEADERS})

source_group("Source Files" FILES ${pnCrash_SOURCES})
source_group("Header Files" FILES ${pnCrash_HEADERS})