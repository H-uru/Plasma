include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

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
