# Skip CMake's built in FindOpenAL module and use OpenAL-Soft's config instead, if possible
find_package(OpenAL CONFIG QUIET)
if(NOT TARGET OpenAL::OpenAL)
    message(DEBUG "OpenAL CMake Config not found, using FindOpenAL module.")
    find_package(OpenAL REQUIRED)
    add_library(OpenAL::OpenAL STATIC IMPORTED)
    set_target_properties(OpenAL::OpenAL PROPERTIES
                          INTERFACE_INCLUDE_DIRECTORIES ${OPENAL_INCLUDE_DIR}
                          IMPORTED_LOCATION ${OPENAL_LIBRARY}
    )
    if(WIN32)
        set_target_properties(OpenAL::OpenAL PROPERTIES
                              INTERFACE_LINK_LIBRARIES WinMM
        )
    endif()
else()
    # Ensure the default RelWithDebInfo and MinSizeRel configs use the Release library with Visual C++.
    # Otherwise, compile-time assertions occur due to iterator debug level
    if(MSVC)
        set_target_properties(OpenAL::OpenAL PROPERTIES
                              MAP_IMPORTED_CONFIG_MINSIZEREL Release
                              MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
        )
    endif()
endif()

set_package_properties(OpenAL PROPERTIES
    URL "https://openal-soft.org/"
    DESCRIPTION "3D audio and EAX library"
    TYPE REQUIRED
)

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
        OpenAL::OpenAL
        CoreLib
        plAnimation
        plAudioCore
        plMessage
)

source_group("Source Files" FILES ${plAudio_SOURCES})
source_group("Header Files" FILES ${plAudio_HEADERS})
