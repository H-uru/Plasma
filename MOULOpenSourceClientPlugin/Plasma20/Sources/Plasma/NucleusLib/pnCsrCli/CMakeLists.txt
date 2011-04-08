include_directories(../../CoreLib)
include_directories(../../NucleusLib)

set(pnCsrCli_SOURCES
    pnCsrCli.cpp
)

set(pnCsrCli_HEADERS
    Pch.h
    pnCsrCli.h
)

add_library(pnCsrCli STATIC ${pnCsrCli_SOURCES} ${pnCsrCli_HEADERS})

source_group("Source Files" FILES ${pnCsrCli_SOURCES})
source_group("Header Files" FILES ${pnCsrCli_HEADERS})