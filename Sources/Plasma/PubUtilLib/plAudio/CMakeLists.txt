set(plAudio_SOURCES
    plAudioEndpointVolume.cpp
    plAudioSystem.cpp
    plDSoundBuffer.cpp
    plEAXEffects.cpp
    plEAXListenerMod.cpp
    plSound.cpp
    plSoundEvent.cpp
    plVoiceChat.cpp
    plVoiceCodec.cpp
    plWin32GroupedSound.cpp
    plWin32Sound.cpp
    plWin32StaticSound.cpp
    plWin32StreamingSound.cpp
    plWin32VideoSound.cpp
)

set(plAudio_HEADERS
    plAudioCreatable.h
    plAudioEndpointVolume.h
    plAudioSystem.h
    plAudioSystem_Private.h
    plDSoundBuffer.h
    plEAXEffects.h
    plEAXListenerMod.h
    plEAXStructures.h
    plSound.h
    plSoundEvent.h
    plVoiceChat.h
    plVoiceCodec.h
    plWin32GroupedSound.h
    plWin32Sound.h
    plWin32StaticSound.h
    plWin32StreamingSound.h
    plWin32VideoSound.h
)

plasma_library(plAudio
    FOLDER PubUtilLib
    SOURCES ${plAudio_SOURCES} ${plAudio_HEADERS}
    UNITY_BUILD
    PRECOMPILED_HEADERS Pch.h
)

target_link_libraries(plAudio
    PUBLIC
        CoreLib
        OpenAL::OpenAL
        pnKeyedObject
        pnModifier
        pnNetCommon
        plAnimation
        plAudioCore
        plMessage
    PRIVATE
        pnMessage
        pnNucleusInc
        pnSceneObject
        plAudible
        plDrawable
        plIntersect
        plMessage
        plPipeline
        plStatusLog
        $<$<BOOL:${USE_OPUS}>:Opus::opus>
        $<$<BOOL:${USE_SPEEX}>:Speex::speex>
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plAudio_SOURCES})
source_group("Header Files" FILES ${plAudio_HEADERS})
