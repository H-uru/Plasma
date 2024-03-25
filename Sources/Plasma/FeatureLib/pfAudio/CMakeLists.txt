set(pfAudio_SOURCES
    plListener.cpp
    plRandomSoundMod.cpp
)

set(pfAudio_HEADERS
    pfAudioCreatable.h
    plListener.h
    plRandomSoundMod.h
)

plasma_library(pfAudio SOURCES ${pfAudio_SOURCES} ${pfAudio_HEADERS})
target_link_libraries(
    pfAudio
    PUBLIC
        CoreLib
        pnModifier
        pfAnimation
    PRIVATE
        pnKeyedObject
        pnMessage
        pnNetCommon
        pnNucleusInc
        pnSceneObject
        plAudio
        plAvatar
        plMessage
        plPipeline
        plStatusLog
        pfCamera
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfAudio_SOURCES})
source_group("Header Files" FILES ${pfAudio_HEADERS})
