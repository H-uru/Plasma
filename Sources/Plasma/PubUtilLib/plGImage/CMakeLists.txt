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

plasma_library(plGImage
    SOURCES ${plGImage_SOURCES} ${plGImage_HEADERS}
    PRECOMPILED_HEADERS Pch.h
)
target_link_libraries(
    plGImage
    PUBLIC
        CoreLib
        pnKeyedObject
        plMessage
        plResMgr
    PRIVATE
        pnDispatch
        pnMessage
        pnNucleusInc
        plMessageBox
        plResMgr
        JPEG::JPEG
        PNG::PNG
        $<$<PLATFORM_ID:Windows>:vfw32>
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plGImage_SOURCES})
source_group("Header Files" FILES ${plGImage_HEADERS})
