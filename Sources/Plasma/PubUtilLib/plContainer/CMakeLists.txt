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

add_library(plContainer STATIC ${plContainer_SOURCES} ${plContainer_HEADERS})
target_link_libraries(
    plContainer
    PUBLIC
        CoreLib
        string_theory
)

source_group("Source Files" FILES ${plContainer_SOURCES})
source_group("Header Files" FILES ${plContainer_HEADERS})
