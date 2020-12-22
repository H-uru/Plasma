plasma_find_dependency(OpenAL FIXUP_TARGET OpenAL::OpenAL REQUIRED)

if(PLASMA_USE_OPUS)
    include_directories(${Opus_INCLUDE_DIR})
endif()
if(PLASMA_USE_SPEEX)
    include_directories(${Speex_INCLUDE_DIR})
endif()

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
        CoreLib
        plAnimation
        plAudioCore
        plMessage

        OpenAL::OpenAL
)

source_group("Source Files" FILES ${plAudio_SOURCES})
source_group("Header Files" FILES ${plAudio_HEADERS})
