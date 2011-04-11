include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

if(WIN32)
	add_definitions(-DWIN32)
endif(WIN32)

set(pfSecurePreloader_SOURCES
	pfSecurePreloader.cpp
)

set(pfSecurePreloader_HEADERS
	pfSecurePreloader.h
	pfSecurePreloaderCreatable.h
)

add_library(pfSecurePreloader STATIC ${pfSecurePreloader_SOURCES} ${pfSecurePreloader_HEADERS})

source_group("Source Files" FILES ${pfSecurePreloader_SOURCES})
source_group("Header Files" FILES ${pfSecurePreloader_HEADERS})
