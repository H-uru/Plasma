set(plNetCommon_SOURCES
    plClientGuid.cpp
    plNetCommon.cpp
    plNetCommonHelpers.cpp
    plNetMember.cpp
    plNetMsgScreener.cpp
    plNetObjectDebugger.cpp
    plNetServerSessionInfo.cpp
    plSpawnPointInfo.cpp
)

set(plNetCommon_HEADERS
    plClientGuid.h
    plNetCommon.h
    plNetCommonCreatable.h
    plNetCommonHelpers.h
    plNetMember.h
    plNetMsgHandler.h
    plNetMsgScreener.h
    plNetObjectDebugger.h
    plNetServerSessionInfo.h
    plSpawnPointInfo.h
)

plasma_library(plNetCommon
    FOLDER PubUtilLib
    SOURCES ${plNetCommon_SOURCES} ${plNetCommon_HEADERS}
    UNITY_BUILD
    PRECOMPILED_HEADERS Pch.h
)

target_include_directories(plNetCommon PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")
target_link_libraries(plNetCommon
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnNetCommon
        pnNetProtocol
        pnUUID
    PRIVATE
        pnMessage
        pnNucleusInc
        plCompression
        plMessage
        plNetClientComm
        plResMgr
        plStatusLog
        pfMessage # pfKIMsg in plNetMsgScreener
)

source_group("Source Files" FILES ${plNetCommon_SOURCES})
source_group("Header Files" FILES ${plNetCommon_HEADERS})
