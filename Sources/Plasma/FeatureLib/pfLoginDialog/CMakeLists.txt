include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

set(pfLoginDialog_SOURCES
    plLoginDialog.cpp
)

set(pfLoginDialog_HEADERS
    plLoginDialog.h
    resource.h
)

set(pfLoginDialog_RESOURCES
    pfLoginDialog.rc
)

add_library(pfLoginDialog STATIC ${pfLoginDialog_SOURCES} ${pfLoginDialog_HEADERS} ${pfLoginDialog_RESOURCES})

source_group("Source Files" FILES ${pfLoginDialog_SOURCES})
source_group("Header Files" FILES ${pfLoginDialog_HEADERS})
source_group("Resource Files" FILES ${pfLoginDialog_RESOURCES})
