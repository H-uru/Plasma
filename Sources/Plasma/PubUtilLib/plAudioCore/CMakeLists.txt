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

add_library(plAudioCore STATIC ${plAudioCore_SOURCES} ${plAudioCore_HEADERS})
target_link_libraries(
    plAudioCore
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
        plStatusLog
        plUnifiedTime

        Ogg::ogg
        Vorbis::vorbis
        Vorbis::vorbisfile
)

if(WIN32)
    # Needed for WAV file support
    target_link_libraries(plAudioCore PRIVATE ${DirectX_LIBRARIES})
endif(WIN32)

source_group("Source Files" FILES ${plAudioCore_SOURCES})
source_group("Header Files" FILES ${plAudioCore_HEADERS})
