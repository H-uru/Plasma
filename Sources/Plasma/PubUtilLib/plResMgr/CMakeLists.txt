include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plResMgr_SOURCES
	plBSDiffBuffer.cpp
	plDiffBuffer.cpp
	plKeyFinder.cpp
	plLocalization.cpp
	plPageInfo.cpp
	plRegistryHelpers.cpp
	plRegistryKeyList.cpp
	plRegistryNode.cpp
	plResManager.cpp
	plResManagerHelper.cpp
	plVersion.cpp
)

set(plResMgr_HEADERS
	plBSDiffBuffer.h
	plDiffBuffer.h
	plKeyFinder.h
	plLocalization.h
	plPageInfo.h
	plRegistryHelpers.h
	plRegistryKeyList.h
	plRegistryNode.h
	plResManager.h
	plResManagerHelper.h
	plResMgrCreatable.h
	plResMgrSettings.h
	plVersion.h
)

add_library(plResMgr STATIC ${plResMgr_SOURCES} ${plResMgr_HEADERS})

source_group("Source Files" FILES ${plResMgr_SOURCES})
source_group("Header Files" FILES ${plResMgr_HEADERS})
