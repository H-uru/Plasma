set(plFontConverter_HEADERS
    plFontConverter.h
    plFontFreeType.h
    plFontPreview.h
)
qt5_wrap_cpp(plFontConverter_MOC ${plFontConverter_HEADERS})

set(plFontConverter_SOURCES
    plFontConverter.cpp
    plFontFreeType.cpp
    plFontPreview.cpp
)

if(WIN32)
    set(plFontConverter_SOURCES ${plFontConverter_SOURCES}
        res/plFontConverter.rc
    )
endif()

set(plFontConverter_RCC_SOURCES
    res/plFontConverter.qrc
)
qt5_add_resources(plFontConverter_RCC ${plFontConverter_RCC_SOURCES})

set(plFontConverter_UIC_SOURCES
    res/MainDialog.ui
    res/FonChooser.ui
    res/FreeType.ui
    res/FreeTypeBatch.ui
)
qt5_wrap_ui(plFontConverter_UIC ${plFontConverter_UIC_SOURCES})

add_executable(plFontConverter WIN32 MACOSX_BUNDLE
               ${plFontConverter_HEADERS} ${plFontConverter_SOURCES}
               ${plFontConverter_RCC} ${plFontConverter_UIC}
               ${plFontConverter_MOC})
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
        Qt5::Widgets
)

# For generated ui_*.h files
# For generated ui_*.h files
target_include_directories(
    plFontConverter
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_LIST_DIR}
)

source_group("Source Files" FILES ${plFontConverter_SOURCES} ${plFontConverter_MOC})
source_group("Header Files" FILES ${plFontConverter_HEADERS})
source_group("Resource Files" FILES ${plFontConverter_RCC_SOURCES} ${plFontConverter_RCC}
                                    ${plFontConverter_UIC_SOURCES} ${plFontConverter_UIC})
