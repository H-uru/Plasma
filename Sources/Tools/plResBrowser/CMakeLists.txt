include_directories("../../Plasma/Apps")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/FeatureLib/inc")
include_directories("../../Plasma/FeatureLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories("../../Plasma/PubUtilLib")

set(plResBrowser_HEADERS
	plResTreeView.h
	plWinRegistryTools.h
)

set(plResBrowser_SOURCES
	plResBrowser.cpp
	plResBrowserWndProc.cpp
	plResTreeView.cpp
	plWinRegistryTools.cpp
	)
	
set(plResBrowser_RESOURCES
	res/plResBrowser.rc
	res/resource.h

	res/dataicon.ico
	res/icon1.ico
	res/icon2.ico
	res/indexico.ico
	res/mergedda.ico
	res/mergedin.ico
	)
	
add_executable(plResBrowser WIN32 ${plResBrowser_SOURCES} ${plResBrowser_HEADERS} ${plResBrowser_RESOURCES})

target_link_libraries(plResBrowser CoreLib)
target_link_libraries(plResBrowser plFile)
target_link_libraries(plResBrowser plMessage)
target_link_libraries(plResBrowser plResMgr)
target_link_libraries(plResBrowser plScene)
target_link_libraries(plResBrowser plStatusLog)
target_link_libraries(plResBrowser plUnifiedTime)
target_link_libraries(plResBrowser pnAddrInfo)
target_link_libraries(plResBrowser pnDispatch)
target_link_libraries(plResBrowser pnFactory)
target_link_libraries(plResBrowser pnKeyedObject)
target_link_libraries(plResBrowser pnMessage)
target_link_libraries(plResBrowser pnNetCommon)
target_link_libraries(plResBrowser pnNucleusInc)
target_link_libraries(plResBrowser pnProduct)
target_link_libraries(plResBrowser pnSceneObject)
target_link_libraries(plResBrowser pnTimer)
target_link_libraries(plResBrowser pnUtils)
target_link_libraries(plResBrowser pnUtilsExe)

if (WIN32)
    target_link_libraries(plResBrowser Rpcrt4)
    target_link_libraries(plResBrowser Version)
    target_link_libraries(plResBrowser Vfw32)
    target_link_libraries(plResBrowser Ws2_32)
    target_link_libraries(plResBrowser winmm)
    target_link_libraries(plResBrowser strmiids)
	target_link_libraries(plResBrowser comctl32) 
	target_link_libraries(plResBrowser shlwapi)
endif(WIN32)

source_group("Source Files" FILES ${plResBrowser_SOURCES})
source_group("Header Files" FILES ${plResBrowser_HEADERS})
source_group("Resource Files" FILES ${plResBrowser_RESOURCES})