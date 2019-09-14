include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

if(WIN32)
    include_directories(${DirectX_INCLUDE_DIR})
endif(WIN32)
include_directories(${OPENAL_INCLUDE_DIR})

if(PLASMA_USE_OPUS)
    include_directories(${Opus_INCLUDE_DIR})
endif()
if(PLASMA_USE_SPEEX)
    include_directories(${Speex_INCLUDE_DIR})
endif()

set(plAudio_SOURCES
    plAudioCaps.cpp
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
    plWinMicLevel.cpp
    plWin32VideoSound.cpp
)

set(plAudio_HEADERS
    plAudioCaps.h
    plAudioCreatable.h
    plAudioSystem.h
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
    plWinMicLevel.h
    plWin32VideoSound.h
)

add_library(plAudio STATIC ${plAudio_SOURCES} ${plAudio_HEADERS})

source_group("Source Files" FILES ${plAudio_SOURCES})
source_group("Header Files" FILES ${plAudio_HEADERS})
