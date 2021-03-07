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
    plAudioSysMsg.cpp
    plCameraMsg.cpp
    plClientMsg.cpp
    plCorrectionMsg.cpp
    plCursorChangeMsg.cpp
    plEnableMsg.cpp
    plEventCallbackMsg.cpp
    plMessage.cpp
    plMessageWithCallbacks.cpp
    plNodeChangeMsg.cpp
    plNotifyMsg.cpp
    plPlayerPageMsg.cpp
    plProxyDrawMsg.cpp
    plRefMsg.cpp
    plRemoteAvatarInfoMsg.cpp
    plSDLModifierMsg.cpp
    plServerReplyMsg.cpp
    plSimulationMsg.cpp
    plSoundMsg.cpp
    plTimeMsg.cpp
    plWarpMsg.cpp
)

plasma_library(pnMessage
    SOURCES ${pnMessage_HEADERS} ${pnMessage_SOURCES}
    PRECOMPILED_HEADERS Pch.h
    UNITY_BUILD
)

target_include_directories(pnMessage PRIVATE "${PLASMA_SOURCE_ROOT}/PubUtilLib")
target_link_libraries(
    pnMessage
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnNetCommon
        pnNucleusInc
    PRIVATE
        plSDL # :(
)

source_group("Header Files" FILES ${pnMessage_HEADERS})
source_group("Source Files" FILES ${pnMessage_SOURCES})
