set(plStatGather_SOURCES
    plAutoProfile.cpp
    plCalculatedProfiles.cpp
    plProfileManagerFull.cpp
)

set(plStatGather_HEADERS
    plAutoProfile.h
    plCalculatedProfiles.h
    plProfileManagerFull.h
    plStatGatherCreatable.h
)

plasma_library(plStatGather
    FOLDER PubUtilLib
    SOURCES ${plStatGather_SOURCES} ${plStatGather_HEADERS}
)
target_link_libraries(
    plStatGather
    PUBLIC
        CoreLib
        pnKeyedObject
    PRIVATE
        pnMessage
        pnNucleusInc
        plAvatar
        plGImage
        plMessage
        plModifier
        plNetClient
        plPipeline
        plStatusLog
        plUnifiedTime
        plVault
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plStatGather_SOURCES})
source_group("Header Files" FILES ${plStatGather_HEADERS})
