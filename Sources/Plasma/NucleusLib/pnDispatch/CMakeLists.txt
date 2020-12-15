include_directories(${PLASMA_SOURCE_ROOT}/CoreLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib/inc)
include_directories(${PLASMA_SOURCE_ROOT}/PubUtilLib)

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
