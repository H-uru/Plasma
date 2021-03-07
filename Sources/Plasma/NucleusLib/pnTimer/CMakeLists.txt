set(pnTimer_HEADERS
    plTimedValue.h
    pnBuildDates.h
)

set(pnTimer_SOURCES
    hsTimer.cpp
    plTimerCallbackManager.cpp
    pnBranchDate.cpp
    pnBuildDates.cpp
)

plasma_library(pnTimer SOURCES ${pnTimer_HEADERS} ${pnTimer_SOURCES})
target_link_libraries(
    pnTimer
    PUBLIC
        CoreLib
    PRIVATE
        pnKeyedObject
        pnMessage
        pnNucleusInc
)

source_group("Header Files" FILES ${pnTimer_HEADERS})
source_group("Source Files" FILES ${pnTimer_SOURCES})
