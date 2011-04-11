include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnProduct_HEADERS
	Pch.h
	pnProduct.h
)

set(pnProduct_PRIVATE
	Private/pnPrAllIncludes.h
	Private/pnPrBranchId.h
	Private/pnPrBranchId.cpp
	Private/pnPrBuildId.h
	Private/pnPrBuildId.cpp
	Private/pnPrBuildString.h
	Private/pnPrBuildString.cpp
	Private/pnPrBuildType.h
	Private/pnPrBuildType.cpp
	Private/pnPrProductId.h
	Private/pnPrProductId.cpp
)

add_library(pnProduct STATIC ${pnProduct_HEADERS} ${pnProduct_PRIVATE})

source_group("Header Files" FILES ${pnProduct_HEADERS})
source_group("Private" FILES ${pnProduct_PRIVATE})
