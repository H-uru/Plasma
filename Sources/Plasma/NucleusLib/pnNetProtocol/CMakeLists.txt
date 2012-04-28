include_directories("../../CoreLib")
include_directories("../../NucleusLib")

if(PLASMA_TARGETS STREQUAL "Server")
    include_directories("../../ServerLib")
endif(PLASMA_TARGETS STREQUAL "Server")

set(pnNetProtocol_HEADERS
    Intern.h
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
    Private/Protocols/Cli2File/pnNpCli2File.cpp
)

set(pnNetProtocol_PROTO_CLI2GAME
    Private/Protocols/Cli2Game/pnNpCli2Game.h
    Private/Protocols/Cli2Game/pnNpCli2Game.cpp
)

set(pnNetProtocol_PROTO_CLI2GK
    Private/Protocols/Cli2GateKeeper/pnNpCli2GateKeeper.h
    Private/Protocols/Cli2GateKeeper/pnNpCli2GateKeeper.cpp
)

set(pnNetProtocol_PROTO_SRV2DB
    Private/Protocols/Srv2Db/pnNpSrv2Db.h
    Private/Protocols/Srv2Db/pnNpSrv2Db.cpp
)

set(pnNetProtocol_PROTO_SRV2LOG
    Private/Protocols/Srv2Log/pnNpSrv2Log.h
    Private/Protocols/Srv2Log/pnNpSrv2Log.cpp
)

set(pnNetProtocol_PROTO_SRV2MCP
    Private/Protocols/Srv2Mcp/pnNpSrv2Mcp.h
    Private/Protocols/Srv2Mcp/pnNpSrv2Mcp.cpp
)

set(pnNetProtocol_PROTO_SRV2SCORE
    Private/Protocols/Srv2Score/pnNpSrv2Score.h
    Private/Protocols/Srv2Score/pnNpSrv2Score.cpp
)

set(pnNetProtocol_PROTO_SRV2STATE
    Private/Protocols/Srv2State/pnNpSrv2State.h
    Private/Protocols/Srv2State/pnNpSrv2State.cpp
)

set(pnNetProtocol_PROTO_SRV2VAULT
    Private/Protocols/Srv2Vault/pnNpSrv2Vault.h
    Private/Protocols/Srv2Vault/pnNpSrv2Vault.cpp
)

add_library(pnNetProtocol STATIC ${pnNetProtocol_HEADERS} ${pnNetProtocol_PRIVATE}
                          ${pnNetProtocol_PROTO_CLI2AUTH} ${pnNetProtocol_PROTO_CLI2FILE}
                          ${pnNetProtocol_PROTO_CLI2GAME} ${pnNetProtocol_PROTO_CLI2GK}
                          ${pnNetProtocol_PROTO_SRV2DB} ${pnNetProtocol_PROTO_SRV2LOG}
                          ${pnNetProtocol_PROTO_SRV2MCP} ${pnNetProtocol_PROTO_SRV2SCORE}
                          ${pnNetProtocol_PROTO_SRV2STATE} ${pnNetProtocol_PROTO_SRV2VAULT}
)

source_group("Header Files" FILES ${pnNetProtocol_HEADERS})
source_group("Private" FILES ${pnNetProtocol_PRIVATE})
source_group("Private\\Cli2Auth" FILES ${pnNetProtocol_PROTO_CLI2AUTH})
source_group("Private\\Cli2File" FILES ${pnNetProtocol_PROTO_CLI2FILE})
source_group("Private\\Cli2Game" FILES ${pnNetProtocol_PROTO_CLI2GAME})
source_group("Private\\Cli2GateKeeper" FILES ${pnNetProtocol_PROTO_CLI2GK})
source_group("Private\\Srv2Db" FILES ${pnNetProtocol_PROTO_SRV2DB})
source_group("Private\\Srv2Log" FILES ${pnNetProtocol_PROTO_SRV2LOG})
source_group("Private\\Srv2Mcp" FILES ${pnNetProtocol_PROTO_SRV2MCP})
source_group("Private\\Srv2Score" FILES ${pnNetProtocol_PROTO_SRV2SCORE})
source_group("Private\\Srv2State" FILES ${pnNetProtocol_PROTO_SRV2STATE})
source_group("Private\\Srv2Vault" FILES ${pnNetProtocol_PROTO_SRV2VAULT})
