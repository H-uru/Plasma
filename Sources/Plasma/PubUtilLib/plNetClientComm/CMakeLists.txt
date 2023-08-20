set(plNetClientComm_SOURCES
    plNetClientComm.cpp
)

set(plNetClientComm_HEADERS
    plNetClientComm.h
    plNetClientCommCreatable.h
)

plasma_library(plNetClientComm SOURCES ${plNetClientComm_SOURCES} ${plNetClientComm_HEADERS})
target_link_libraries(
    plNetClientComm
    PUBLIC
        CoreLib
        pnEncryption
        pnNetBase
        pnUUID
        plMessage
        plNetCommon
    PRIVATE
        pnAsyncCore
        pnNetCli
        pnNetCommon
        pnNucleusInc
        pnUtils
        plFile
        plNetGameLib
        plNetMessage
        plVault
)

target_include_directories(plNetClientComm PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")

source_group("Source Files" FILES ${plNetClientComm_SOURCES})
source_group("Header Files" FILES ${plNetClientComm_HEADERS})
