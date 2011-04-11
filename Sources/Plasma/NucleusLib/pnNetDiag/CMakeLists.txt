include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnNetDiag_HEADERS
    Intern.h
    Pch.h
    pnNetDiag.h
)

set(pnNetDiag_SOURCES
    pnNdDns.cpp
    pnNdIcmp.cpp
    pnNdTcp.cpp
    pnNetDiag.cpp
    pnNetSys.cpp
)

add_library(pnNetDiag STATIC ${pnNetDiag_HEADERS} ${pnNetDiag_SOURCES})

source_group("Header Files" FILES ${pnNetDiag_HEADERS})
source_group("Source Files" FILES ${pnNetDiag_SOURCES})
