include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnMail_HEADERS
    Pch.h
    pnMail.h
)

set(pnMail_SOURCES
    pnMail.cpp
)

add_library(pnMail STATIC ${pnMail_HEADERS} ${pnMail_SOURCES})

source_group("Header Files" FILES ${pnMail_HEADERS})
source_group("Source Files" FILES ${pnMail_SOURCES})
