set(pnNetProtocol_HEADERS
    pnNpCli2Auth.h
    pnNpCli2File.h
    pnNpCli2Game.h
    pnNpCli2GateKeeper.h
    pnNpCommon.h
)

set(pnNetProtocol_SOURCES
    pnNpCli2Auth.cpp
    pnNpCli2Game.cpp
    pnNpCli2GateKeeper.cpp
    pnNpCommon.cpp
)

plasma_library(pnNetProtocol
    FOLDER NucleusLib
    SOURCES ${pnNetProtocol_HEADERS} ${pnNetProtocol_SOURCES}
)
target_link_libraries(
    pnNetProtocol
    PUBLIC
        CoreLib
        pnEncryption
        pnNetBase
        pnNetCli
        pnUUID
    PRIVATE
        pnUtils
)

source_group("Header Files" FILES ${pnNetProtocol_HEADERS})
source_group("Source Files" FILES ${pnNetProtocol_SOURCES})
