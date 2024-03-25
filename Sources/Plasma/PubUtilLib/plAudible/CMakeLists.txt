set(plAudible_SOURCES
    plAudibleNull.cpp
    plWinAudible.cpp
    plWinAudibleProxy.cpp
)

set(plAudible_HEADERS
    plAudibleCreatable.h
    plAudibleNull.h
    plWinAudible.h
    plWinAudibleProxy.h
)

plasma_library(plAudible SOURCES ${plAudible_SOURCES} ${plAudible_HEADERS})
target_link_libraries(
    plAudible
    PUBLIC
        CoreLib
        pnNucleusInc
        plDrawable
    PRIVATE
        pnInputCore
        pnKeyedObject
        pnMessage
        pnSceneObject
        plAudio
        plMessage
        plModifier
        plStatusLog
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plAudible_SOURCES})
source_group("Header Files" FILES ${plAudible_HEADERS})
