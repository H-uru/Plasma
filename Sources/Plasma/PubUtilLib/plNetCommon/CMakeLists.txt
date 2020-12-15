include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

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
    plNetCommonConstants.h
    plNetCommonCreatable.h
    plNetCommonHelpers.h
    plNetMember.h
    plNetMsgHandler.h
    plNetMsgScreener.h
    plNetObjectDebugger.h
    plNetServerSessionInfo.h
    plSpawnPointInfo.h
)

add_library(plNetCommon STATIC ${plNetCommon_SOURCES} ${plNetCommon_HEADERS})

source_group("Source Files" FILES ${plNetCommon_SOURCES})
source_group("Header Files" FILES ${plNetCommon_HEADERS})
