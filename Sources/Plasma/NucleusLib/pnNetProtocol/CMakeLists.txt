set(pnNetProtocol_HEADERS
    Pch.h
    pnNetProtocol.h
)

set(pnNetProtocol_PRIVATE
    Private/pnNpAllIncludes.h
    Private/pnNpCommon.h
    Private/pnNpCommon.cpp
)

set(pnNetProtocol_PROTO_CLI2AUTH
    Private/Protocols/Cli2Auth/pnNpCli2Auth.h
    Private/Protocols/Cli2Auth/pnNpCli2Auth.cpp
)

set(pnNetProtocol_PROTO_CLI2FILE
    Private/Protocols/Cli2File/pnNpCli2File.h
)

set(pnNetProtocol_PROTO_CLI2GAME
    Private/Protocols/Cli2Game/pnNpCli2Game.h
    Private/Protocols/Cli2Game/pnNpCli2Game.cpp
)

set(pnNetProtocol_PROTO_CLI2GK
    Private/Protocols/Cli2GateKeeper/pnNpCli2GateKeeper.h
    Private/Protocols/Cli2GateKeeper/pnNpCli2GateKeeper.cpp
)

plasma_library(pnNetProtocol
    FOLDER NucleusLib
    SOURCES
        ${pnNetProtocol_HEADERS} ${pnNetProtocol_PRIVATE}
        ${pnNetProtocol_PROTO_CLI2AUTH} ${pnNetProtocol_PROTO_CLI2FILE}
        ${pnNetProtocol_PROTO_CLI2GAME} ${pnNetProtocol_PROTO_CLI2GK}
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
source_group("Private" FILES ${pnNetProtocol_PRIVATE})
source_group("Private\\Cli2Auth" FILES ${pnNetProtocol_PROTO_CLI2AUTH})
source_group("Private\\Cli2File" FILES ${pnNetProtocol_PROTO_CLI2FILE})
source_group("Private\\Cli2Game" FILES ${pnNetProtocol_PROTO_CLI2GAME})
source_group("Private\\Cli2GateKeeper" FILES ${pnNetProtocol_PROTO_CLI2GK})
