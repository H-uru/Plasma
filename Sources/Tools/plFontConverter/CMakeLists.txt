include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/PubUtilLib")
include_directories(${FREETYPE_INCLUDE_DIRS})

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

# For generated ui_*.h files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(plFontConverter WIN32 MACOSX_BUNDLE
               ${plFontConverter_HEADERS} ${plFontConverter_SOURCES}
               ${plFontConverter_RCC} ${plFontConverter_UIC}
               ${plFontConverter_MOC})

target_link_libraries(plFontConverter CoreLib)
target_link_libraries(plFontConverter pnKeyedObject)
target_link_libraries(plFontConverter pnSceneObject)
target_link_libraries(plFontConverter plGImage)
target_link_libraries(plFontConverter plPipeline)
target_link_libraries(plFontConverter plResMgr)
target_link_libraries(plFontConverter ${JPEG_LIBRARY})
target_link_libraries(plFontConverter ${FREETYPE_LIBRARIES})
target_link_libraries(plFontConverter Qt5::Widgets)

if(USE_VLD)
    target_link_libraries(plFontConverter ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plFontConverter_SOURCES} ${plFontConverter_MOC})
source_group("Header Files" FILES ${plFontConverter_HEADERS})
source_group("Resource Files" FILES ${plFontConverter_RCC_SOURCES} ${plFontConverter_RCC}
                                    ${plFontConverter_UIC_SOURCES} ${plFontConverter_UIC})
