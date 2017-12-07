include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(pnMessage_HEADERS
    plAttachMsg.h
    plAudioSysMsg.h
    plCameraMsg.h
    plClientMsg.h
    plCmdIfaceModMsg.h
    plCorrectionMsg.h
    plCursorChangeMsg.h
    plDISpansMsg.h
    plEnableMsg.h
    plEventCallbackMsg.h
    plFakeOutMsg.h
    plIntRefMsg.h
    plMessage.h
    plMessageWithCallbacks.h
    plMultiModMsg.h
    plNodeChangeMsg.h
    plNodeRefMsg.h
    plNotifyMsg.h
    plObjRefMsg.h
    plPipeResMakeMsg.h
    plPlayerPageMsg.h
    plProxyDrawMsg.h
    plRefMsg.h
    plRemoteAvatarInfoMsg.h
    plSatisfiedMsg.h
    plSDLModifierMsg.h
    plSDLNotificationMsg.h
    plSelfDestructMsg.h
    plServerReplyMsg.h
    plSetNetGroupIDMsg.h
    plSharedStateMsg.h
    plSimulationMsg.h
    plSimulationSynchMsg.h
    plSingleModMsg.h
    plSoundMsg.h
    plTimeMsg.h
    plWarpMsg.h
    pnMessageCreatable.h
)

set(pnMessage_SOURCES
    plCameraMsg.cpp
    plClientMsg.cpp
    plMessage.cpp
    plMessageWithCallbacks.cpp
    plNodeChangeMsg.cpp
    plNotifyMsg.cpp
    plProxyDrawMsg.cpp
    plRefMsg.cpp
    plSDLModifierMsg.cpp
    plServerReplyMsg.cpp
    plSimulationMsg.cpp
    plSoundMsg.cpp
    plTimeMsg.cpp
)

add_library(pnMessage STATIC ${pnMessage_HEADERS} ${pnMessage_SOURCES})
target_link_libraries(pnMessage CoreLib pnNetCommon)

source_group("Header Files" FILES ${pnMessage_HEADERS})
source_group("Source Files" FILES ${pnMessage_SOURCES})
