set(plFontConverter_HEADERS
    plFontConverter.h
    plFontFreeType.h
    plFontPreview.h
)

set(plFontConverter_SOURCES
    plFontConverter.cpp
    plFontFreeType.cpp
    plFontPreview.cpp
)

if(WIN32)
    list(APPEND plFontConverter_SOURCES
        res/plFontConverter.rc
    )
endif()

set(plFontConverter_RCC
    res/plFontConverter.qrc
)

set(plFontConverter_UIC
    res/MainDialog.ui
    res/FonChooser.ui
    res/FreeType.ui
    res/FreeTypeBatch.ui
)

plasma_executable(plFontConverter TOOL QT_GUI
    SOURCES
        ${plFontConverter_HEADERS} ${plFontConverter_SOURCES}
        ${plFontConverter_RCC} ${plFontConverter_UIC}
)
target_link_libraries(
    plFontConverter
    PRIVATE
        CoreLib
        pnKeyedObject
        pnNucleusInc
        plGImage
        plMessage
        plResMgr
        freetype
        Qt::Widgets
)

source_group("Source Files" FILES ${plFontConverter_SOURCES} ${plFontConverter_MOC})
source_group("Header Files" FILES ${plFontConverter_HEADERS})
source_group("Resource Files" FILES ${plFontConverter_RCC} ${plFontConverter_UIC})
