include_directories("../../CoreLib")
include_directories("../../NucleusLib")

set(pnIniExe_HEADERS
	Pch.h
)

set(pnIniExe_PRIVATE
	Private/pnIniSrv.cpp
)

set(pnIniExe_PRIVATE_WIN32
	Private/Win32/pnW32IniChange.cpp
)

add_library(pnIniExe STATIC ${pnIniExe_HEADERS} ${pnIniExe_PRIVATE} ${pnIniExe_PRIVATE_WIN32})

source_group("Header Files" FILES ${pnIniExe_HEADERS})
source_group("Private" FILES ${pnIniExe_PRIVATE})
source_group("Private\\Win32" FILES ${pnIniExe_PRIVATE_WIN32})
