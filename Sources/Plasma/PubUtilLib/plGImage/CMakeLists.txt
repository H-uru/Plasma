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

target_link_libraries(
    plGImage
    PUBLIC
        CoreLib
        pnKeyedObject
        plMessage
        plResMgr
    PRIVATE
        JPEG::JPEG
        PNG::PNG
)

source_group("Source Files" FILES ${plGImage_SOURCES})
source_group("Header Files" FILES ${plGImage_HEADERS})
