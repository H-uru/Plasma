include_directories("../../CoreLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

find_package(JPEG REQUIRED)
include_directories(${JPEG_INCLUDE_DIR})

set(plJPEG_SOURCES
	plJPEG.cpp
)

set(plJPEG_HEADERS
	plJPEG.h
)

add_library(plJPEG STATIC ${plJPEG_SOURCES} ${plJPEG_HEADERS})

source_group("Source Files" FILES ${plJPEG_SOURCES})
source_group("Header Files" FILES ${plJPEG_HEADERS})
