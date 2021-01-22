# Because there are no CPP sources, this needs to be an interface library. Change to
# a static library if you ever add any real C++ code.
add_library(plPubUtilInc INTERFACE)
set_property(
    TARGET plPubUtilInc
    PROPERTY INTERFACE_LINK_LIBRARIES
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
set_property(
    TARGET plPubUtilInc
    PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}"
)
