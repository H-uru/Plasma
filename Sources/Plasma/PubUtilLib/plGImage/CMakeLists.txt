include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${JPEG_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})

set(plGImage_SOURCES
    hsCodecManager.cpp
    hsDXTDirectXCodec.cpp
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
    hsCodecManager.h
    hsDXTDirectXCodec.h
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
