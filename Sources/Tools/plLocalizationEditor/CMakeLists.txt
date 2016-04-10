include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/PubUtilLib")
include_directories("../../Plasma/FeatureLib")

add_definitions("-DUNICODE")

set(plLocalizationEditor_HEADERS
    plAddDlgs.h
    plEditDlg.h
    plLocTreeView.h
)
qt5_wrap_cpp(plLocalizationEditor_MOC ${plLocalizationEditor_HEADERS})

set(plLocalizationEditor_SOURCES
    plAddDlgs.cpp
    plEditDlg.cpp
    plLocalizationEditor.cpp
    plLocTreeView.cpp
)

if(WIN32)
    set(plLocalizationEditor_SOURCES ${plLocalizationEditor_SOURCES}
        res/plLocalizationEditor.rc
    )
endif()

set(plLocalizationEditor_RCC_SOURCES
    res/plLocalizationEditor.qrc
)
qt5_add_resources(plLocalizationEditor_RCC ${plLocalizationEditor_RCC_SOURCES})

set(plLocalizationEditor_UIC_SOURCES
    res/EditDialog.ui
    res/AddElement.ui
    res/AddLocalization.ui
)
qt5_wrap_ui(plLocalizationEditor_UIC ${plLocalizationEditor_UIC_SOURCES})

# For generated ui_*.h files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(plLocalizationEditor WIN32 MACOSX_BUNDLE
               ${plLocalizationEditor_HEADERS} ${plLocalizationEditor_SOURCES}
               ${plLocalizationEditor_RCC} ${plLocalizationEditor_UIC}
               ${plLocalizationEditor_MOC})

target_link_libraries(plLocalizationEditor CoreLib)
target_link_libraries(plLocalizationEditor pnSceneObject)
target_link_libraries(plLocalizationEditor pnUUID)
target_link_libraries(plLocalizationEditor plResMgr)
target_link_libraries(plLocalizationEditor pfLocalizationMgr)
target_link_libraries(plLocalizationEditor ${EXPAT_LIBRARY})
target_link_libraries(plLocalizationEditor ${STRING_THEORY_LIBRARIES})
target_link_libraries(plLocalizationEditor Qt5::Widgets)

if(USE_VLD)
    target_link_libraries(plLocalizationEditor ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plLocalizationEditor_SOURCES} ${plLocalizationEditor_MOC})
source_group("Header Files" FILES ${plLocalizationEditor_HEADERS})
source_group("Resource Files" FILES ${plLocalizationEditor_RCC_SOURCES} ${plLocalizationEditor_RCC}
                                    ${plLocalizationEditor_UIC_SOURCES} ${plLocalizationEditor_UIC})
