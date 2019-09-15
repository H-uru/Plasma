option(USE_OPUS "Use opus audio, if available" ON)
option(USE_SPEEX "Use speex voice chat, if avalable" ON)
option(USE_VPX "Use VPX movie support, if available" ON)

if(USE_OPUS AND Opus_FOUND)
    set(PLASMA_USE_OPUS TRUE)
endif()

if(USE_SPEEX AND Speex_FOUND)
    set(PLASMA_USE_SPEEX TRUE)
endif()

if(USE_VPX AND VPX_FOUND)
    set(PLASMA_USE_VPX TRUE)
endif()

if(PLASMA_USE_OPUS AND PLASMA_USE_VPX)
    set(PLASMA_USE_WEBM TRUE)
endif()
