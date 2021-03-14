set(pnDispatch_SOURCES
    plDispatch.cpp
    plDispatchLogBase.cpp
)

set(pnDispatch_HEADERS
    plDispatch.h
    plDispatchLogBase.h
    pnDispatchCreatable.h
)

plasma_library(pnDispatch SOURCES ${pnDispatch_SOURCES} ${pnDispatch_HEADERS})
target_link_libraries(
    pnDispatch
    PUBLIC
        CoreLib
        pnFactory
        pnKeyedObject
        pnNucleusInc
    PRIVATE
        pnMessage
        pnNetCommon
)

source_group("Source Files" FILES ${pnDispatch_SOURCES})
source_group("Header Files" FILES ${pnDispatch_HEADERS})
