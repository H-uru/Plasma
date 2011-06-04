include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnNetCli_HEADERS
    Intern.h
    Pch.h
    pnNetCli.h
)

set(pnNetCli_SOURCES
    pnNcChannel.cpp
    pnNcCli.cpp
    pnNcEncrypt.cpp
    pnNcUtils.cpp
)

add_library(pnNetCli STATIC ${pnNetCli_HEADERS} ${pnNetCli_SOURCES})

source_group("Header Files" FILES ${pnNetCli_HEADERS})
source_group("Source Files" FILES ${pnNetCli_SOURCES})
