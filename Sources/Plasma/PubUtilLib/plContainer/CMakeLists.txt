set(plContainer_SOURCES
    hsStringTable.cpp
    plConfigInfo.cpp
    plConfigInfoLogging.cpp
    plKeysAndValues.cpp
)

set(plContainer_HEADERS
    hsStringTable.h
    plConfigInfo.h
    plContainer.h
    plKeysAndValues.h
)

plasma_library(plContainer
    FOLDER PubUtilLib
    SOURCES ${plContainer_SOURCES} ${plContainer_HEADERS}
)
target_link_libraries(
    plContainer
    PUBLIC
        CoreLib
)

source_group("Source Files" FILES ${plContainer_SOURCES})
source_group("Header Files" FILES ${plContainer_HEADERS})
