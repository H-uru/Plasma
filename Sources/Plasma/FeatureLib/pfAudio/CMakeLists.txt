include_directories(../../CoreLib)
include_directories(../../FeatureLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)
#include_directories(../../PubUtilLib/inc)

include_directories(${OPENAL_INCLUDE_DIR})

set(pfAudio_SOURCES
    plListener.cpp
    plRandomSoundMod.cpp
)

set(pfAudio_HEADERS
    pfAudioCreatable.h
    plListener.h
    plRandomSoundMod.h
)

add_library(pfAudio STATIC ${pfAudio_SOURCES} ${pfAudio_HEADERS})

source_group("Source Files" FILES ${pfAudio_SOURCES})
source_group("Header Files" FILES ${pfAudio_HEADERS})
