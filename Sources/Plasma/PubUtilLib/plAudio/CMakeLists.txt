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

add_library(plAudio STATIC ${plAudio_SOURCES} ${plAudio_HEADERS})

target_link_libraries(plAudio
    PUBLIC
        OpenAL::OpenAL
        CoreLib
        plAnimation
        plAudioCore
        plMessage
)

if(USE_OPUS)
    target_link_libraries(plAudio PRIVATE Opus::opus)
endif()
if(USE_SPEEX)
    target_link_libraries(plAudio PRIVATE Speex::speex)
endif()

source_group("Source Files" FILES ${plAudio_SOURCES})
source_group("Header Files" FILES ${plAudio_HEADERS})
