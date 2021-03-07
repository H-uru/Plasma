set(plAudioCore_SOURCES
    plAudioFileReader.cpp
    plBufferedFileReader.cpp
    plCachedFileReader.cpp
    plFastWavReader.cpp
    plOGGCodec.cpp
    plSoundBuffer.cpp
    plSoundDeswizzler.cpp
    plWavFile.cpp
)

set(plAudioCore_HEADERS
    plAudioCore.h
    plAudioCoreCreatable.h
    plAudioFileReader.h
    plBufferedFileReader.h
    plCachedFileReader.h
    plFastWavReader.h
    plOGGCodec.h
    plSoundBuffer.h
    plSoundDeswizzler.h
    plWavFile.h
)

plasma_library(plAudioCore SOURCES ${plAudioCore_SOURCES} ${plAudioCore_HEADERS})
target_link_libraries(
    plAudioCore
    PUBLIC
        CoreLib
        pnKeyedObject
    PRIVATE
        pnMessage
        pnNucleusInc

        $<$<PLATFORM_ID:Windows>:${DirectX_LIBRARIES}>
        Ogg::ogg
        Vorbis::vorbis
        Vorbis::vorbisfile
        $<$<PLATFORM_ID:Windows>:winmm>
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plAudioCore_SOURCES})
source_group("Header Files" FILES ${plAudioCore_HEADERS})
