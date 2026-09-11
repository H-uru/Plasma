set(pnNetCli_HEADERS
    Intern.h
    pnNetCli.h
)

set(pnNetCli_SOURCES
    pnNcChannel.cpp
    pnNcCli.cpp
    pnNcEncrypt.cpp
    pnNcUtils.cpp
)

plasma_library(pnNetCli
    FOLDER NucleusLib
    SOURCES ${pnNetCli_HEADERS} ${pnNetCli_SOURCES}
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(
    pnNetCli
    PUBLIC
        CoreLib
        pnNetBase
    PRIVATE
        pnAsyncCore
        pnEncryption
)

source_group("Header Files" FILES ${pnNetCli_HEADERS})
source_group("Source Files" FILES ${pnNetCli_SOURCES})
