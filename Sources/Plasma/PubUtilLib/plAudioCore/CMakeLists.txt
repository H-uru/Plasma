include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${Ogg_INCLUDE_DIR})
include_directories(${Vorbis_INCLUDE_DIR})
# We only need DirectX if we're building the Max plugin
# TODO: Check if we actually need this at all
if(3dsm_BUILD_PLUGIN)
    include_directories(${DirectX_INCLUDE_DIR})
endif(3dsm_BUILD_PLUGIN)

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
target_link_libraries(plAudioCore ${Ogg_LIBRARIES})
target_link_libraries(plAudioCore ${Vorbis_LIBRARIES})

source_group("Source Files" FILES ${plAudioCore_SOURCES})
source_group("Header Files" FILES ${plAudioCore_HEADERS})
