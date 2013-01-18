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

set(plLocalizationEditor_SOURCES
    plAddDlgs.cpp
	plEditDlg.cpp
	plLocalizationEditor.cpp
	plLocTreeView.cpp
)
    
set(plLocalizationEditor_RESOURCES
    res/plLocalizationEditor.rc
    res/resource.h
    res/icon1.ico
)

add_executable(plLocalizationEditor WIN32 ${plLocalizationEditor_HEADERS} ${plLocalizationEditor_SOURCES} ${plLocalizationEditor_RESOURCES})

target_link_libraries(plLocalizationEditor CoreLib)
target_link_libraries(plLocalizationEditor pnSceneObject)
target_link_libraries(plLocalizationEditor pnUUID)
target_link_libraries(plLocalizationEditor plResMgr)
target_link_libraries(plLocalizationEditor pfLocalizationMgr)
target_link_libraries(plLocalizationEditor ${EXPAT_LIBRARY})
target_link_libraries(plLocalizationEditor comctl32)

if(USE_VLD)
    target_link_libraries(plLocalizationEditor ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plLocalizationEditor_SOURCES})
source_group("Header Files" FILES ${plLocalizationEditor_HEADERS})
source_group("Resource Files" FILES ${plLocalizationEditor_RESOURCES})
