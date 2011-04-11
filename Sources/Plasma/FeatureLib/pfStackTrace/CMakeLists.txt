include_directories(../../CoreLib)

if(WIN32)
	add_definitions(-DWIN32)
endif(WIN32)

set(pfStackTrace_SOURCES
	pfMapFile.cpp
	pfMapFileEntry.cpp
	pfPrintStackTrace.cpp
	pfStackTrace.cpp
	pfTextFile.cpp
)

set(pfStackTrace_HEADERS
	pfArray.h
	pfMapFile.h
	pfMapFileEntry.h
	pfPrintStackTrace.h
	pfStackTrace.h
	pfTextFile.h
)

add_library(pfStackTrace STATIC ${pfStackTrace_SOURCES} ${pfStackTrace_HEADERS})

source_group("Source Files" FILES ${pfStackTrace_SOURCES})
source_group("Header Files" FILES ${pfStackTrace_HEADERS})
