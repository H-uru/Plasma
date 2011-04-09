include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnNetBase_HEADERS
	Intern.h
	Pch.h
	pnNbAuthKey.hpp
	pnNbCsrKey.hpp
	pnNbConst.h
	pnNbGameKey.hpp
	pnNbGateKeeperKey.hpp
	pnNetBase.h
)

set(pnNetBase_PRIVATE
	Private/pnNbAllIncludes.h
	Private/pnNbEchoMsgs.h
	Private/pnNbError.h
	Private/pnNbError.cpp
	Private/pnNbProtocol.h
	Private/pnNbProtocol.cpp
	Private/pnNbSrvs.h
	Private/pnNbSrvs.cpp
)

add_library(pnNetBase STATIC ${pnNetBase_HEADERS} ${pnNetBase_PRIVATE})

source_group("Header Files" FILES ${pnNetBase_HEADERS})
source_group("Private" FILES ${pnNetBase_PRIVATE})
