include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../NucleusLib/inc")

set(pnTimer_HEADERS
	plTimedValue.h
	plTimerCallbackManager.h
	pnBuildDates.h
	pnTimerCreatable.h
)

set(pnTimer_SOURCES
	hsTimer.cpp
	plTimedValue.cpp
	plTimerCallbackManager.cpp
	pnBranchDate.cpp
	pnBuildDates.cpp
)

add_library(pnTimer STATIC ${pnTimer_HEADERS} ${pnTimer_SOURCES})

source_group("Header Files" FILES ${pnTimer_HEADERS})
source_group("Source Files" FILES ${pnTimer_SOURCES})
