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

plasma_library(pnNetCli SOURCES ${pnNetCli_HEADERS} ${pnNetCli_SOURCES})
target_link_libraries(
    pnNetCli
    PUBLIC
        CoreLib
        pnEncryption
    PRIVATE
        pnAsyncCore
        pnNetBase
        pnUtils
        pnUUID
)

source_group("Header Files" FILES ${pnNetCli_HEADERS})
source_group("Source Files" FILES ${pnNetCli_SOURCES})
