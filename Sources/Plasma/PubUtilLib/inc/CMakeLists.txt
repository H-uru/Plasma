set(plPubUtilInc_HEADERS
    plAllCreatables.h
)

plasma_library(plPubUtilInc OBJECT SOURCES ${plPubUtilInc_HEADERS})
target_link_libraries(plPubUtilInc
    PUBLIC
        plAgeLoader
        plAnimation
        plAudible
        plAudio
        plAudioCore
        plAvatar
        plDrawable
        plGImage
        plGLight
        plInputCore
        plInterp
        plIntersect
        plMessage
        plModifier
        plNetClient
        plNetClientComm
        plNetCommon
        plNetMessage
        plParticleSystem
        plPipeline
        plPhysical
        plPhysX
        plResMgr
        plScene
        plSDL
        plStatGather
        plSurface
        plVault
)
target_include_directories(plPubUtilInc INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
