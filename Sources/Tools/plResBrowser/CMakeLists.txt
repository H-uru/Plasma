include_directories("../../Plasma/Apps")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/FeatureLib/inc")
include_directories("../../Plasma/FeatureLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories("../../Plasma/PubUtilLib")

set(plResBrowser_SOURCES
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
							  
target_link_libraries(plResBrowser CoreLib CoreLibExe plFile plUnifiedTime pnProduct pnUtils pnUtilsExe)
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
source_group("Header Files" FILES ${plClient_HEADERS})
source_group("Resource Files" FILES ${plClient_RESOURCES})