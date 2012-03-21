include_directories("../../CoreLib")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plNetClient_SOURCES
    plLinkEffectsMgr.cpp
    plNetCliAgeJoiner.cpp
    plNetCliAgeLeaver.cpp
    plNetClientCommInterface.cpp
    plNetClientGroup.cpp
    plNetClientMgr.cpp
    plNetClientMgrLoad.cpp
    plNetClientMgrRecord.cpp
    plNetClientMgrSend.cpp
    plNetClientMgrShow.cpp
    plNetClientMgrVoice.cpp
    plNetClientMsgHandler.cpp
    plNetClientMsgScreener.cpp
    plNetLinkingMgr.cpp
    plNetObjectDebugger.cpp
    plNetVoiceList.cpp
)

set(plNetClient_HEADERS
    plLinkEffectsMgr.h
    plNetCliAgeJoiner.h
    plNetCliAgeLeaver.h
    plNetClientCreatable.h
    plNetClientGroup.h
    plNetClientMgr.h
    plNetClientMsgHandler.h
    plNetClientMsgScreener.h
    plNetLinkingMgr.h
    plNetObjectDebugger.h
    plNetVoiceList.h
)

add_library(plNetClient STATIC ${plNetClient_SOURCES} ${plNetClient_HEADERS})

source_group("Source Files" FILES ${plNetClient_SOURCES})
source_group("Header Files" FILES ${plNetClient_HEADERS})
