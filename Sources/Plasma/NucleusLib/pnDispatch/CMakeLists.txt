include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

set(pnDispatch_SOURCES
    plDispatch.cpp
    plDispatchLogBase.cpp
)

set(pnDispatch_HEADERS
    plDispatch.h
    plDispatchLogBase.h
    pnDispatchCreatable.h
)

add_library(pnDispatch STATIC ${pnDispatch_SOURCES} ${pnDispatch_HEADERS})
target_link_libraries(pnDispatch pnNucleusInc)

source_group("Source Files" FILES ${pnDispatch_SOURCES})
source_group("Header Files" FILES ${pnDispatch_HEADERS})
