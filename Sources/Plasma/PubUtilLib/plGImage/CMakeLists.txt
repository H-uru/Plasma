include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

find_package(PNG REQUIRED)
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
    plDynSurfaceWriter.cpp
    plFont.cpp
    plFontCache.cpp
    plLODMipmap.cpp
    plMipmap.cpp
    plPNG.cpp
    plTGAWriter.cpp
    plWinFontCache.cpp
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
    plDynSurfaceWriter.h
    plFont.h
    plFontCache.h
    plGImageCreatable.h
    plLODMipmap.h
    plMipmap.h
    plPNG.h
    plTGAWriter.h
    plWinFontCache.h
)

add_library(plGImage STATIC ${plGImage_SOURCES} ${plGImage_HEADERS})

source_group("Source Files" FILES ${plGImage_SOURCES})
source_group("Header Files" FILES ${plGImage_HEADERS})