include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

include_directories(${JPEG_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})

set(plGImage_SOURCES
    hsCodecManager.cpp
    hsDXTSoftwareCodec.cpp
    plAVIWriter.cpp
    plBitmap.cpp
    plBumpMapGen.cpp
    plCubicEnvironmap.cpp
    plDynamicTextMap.cpp
    plFont.cpp
    plFontCache.cpp
    plJPEG.cpp
    plLODMipmap.cpp
    plMipmap.cpp
    plPNG.cpp
    plTGAWriter.cpp
)

set(plGImage_HEADERS
    hsCodec.h
    hsCodecManager.h
    hsDXTSoftwareCodec.h
    plAVIWriter.h
    plBitmap.h
    plBumpMapGen.h
    plCubicEnvironmap.h
    plDynamicTextMap.h
    plFont.h
    plFontCache.h
    plGImageCreatable.h
    plJPEG.h
    plLODMipmap.h
    plMipmap.h
    plPNG.h
    plTGAWriter.h
)

add_library(plGImage STATIC ${plGImage_SOURCES} ${plGImage_HEADERS})

source_group("Source Files" FILES ${plGImage_SOURCES})
source_group("Header Files" FILES ${plGImage_HEADERS})
