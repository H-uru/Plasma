set(plLocalizationEditor_HEADERS
    plAddDlgs.h
    plEditDlg.h
    plLocTreeView.h
)

set(plLocalizationEditor_SOURCES
    plAddDlgs.cpp
    plEditDlg.cpp
    plLocalizationEditor.cpp
    plLocTreeView.cpp
)

if(WIN32)
    list(APPEND plLocalizationEditor_SOURCES
        res/plLocalizationEditor.rc
    )
endif()

set(plLocalizationEditor_RCC
    res/plLocalizationEditor.qrc
)

set(plLocalizationEditor_UIC
    res/EditDialog.ui
    res/AddElement.ui
    res/AddLocalization.ui
)

plasma_executable(plLocalizationEditor TOOL QT_GUI
    FOLDER Tools
    SOURCES
        ${plLocalizationEditor_HEADERS} ${plLocalizationEditor_SOURCES}
        ${plLocalizationEditor_RCC} ${plLocalizationEditor_UIC}
)
target_link_libraries(
    plLocalizationEditor
    PRIVATE
        CoreLib
        pnNucleusInc
        plMessage
        plResMgr
        pfLocalizationMgr
        Qt::Widgets
)

source_group("Source Files" FILES ${plLocalizationEditor_SOURCES} ${plLocalizationEditor_MOC})
source_group("Header Files" FILES ${plLocalizationEditor_HEADERS})
source_group("Resource Files" FILES ${plLocalizationEditor_RCC} ${plLocalizationEditor_UIC})
