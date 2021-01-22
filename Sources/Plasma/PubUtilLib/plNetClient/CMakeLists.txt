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
    plNetVoiceList.h
)

add_library(plNetClient STATIC ${plNetClient_SOURCES} ${plNetClient_HEADERS})

target_include_directories(plNetClient PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")
target_link_libraries(plNetClient
    PUBLIC
        CoreLib
        pnKeyedObject
        pnNetCommon
        plMessage # plNetLinkingMgr
        plNetClientComm
        plNetCommon
        plNetTransport
        plUnifiedTime
        string_theory
    PRIVATE
        pnAsyncCore
        pnDispatch
        pnMessage
        pnModifier
        pnNucleusInc
        pnSceneObject
        plAgeDescription
        plAgeLoader
        plAnimation
        plAvatar
        plContainer
        plDrawable
        plMessage
        plModifier
        plNetClientRecorder
        plNetGameLib
        plNetMessage
        plParticleSystem
        plPipeline
        plProgressMgr
        plResMgr
        plScene
        plSDL
        plStatusLog
        plSurface
        plVault
        pfMessage # For pfKIMsg
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plNetClient_SOURCES})
source_group("Header Files" FILES ${plNetClient_HEADERS})
