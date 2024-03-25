list(APPEND plWinDpi_HEADERS
    plWinDpi.h
)

list(APPEND plWinDpi_SOURCES
    plWinDpi.cpp
)

plasma_library(plWinDpi
    SOURCES ${plWinDpi_SOURCES} ${plWinDpi_HEADERS}
)

target_link_libraries(
    plWinDpi
    PUBLIC
        CoreLib
        plStatusLog
)
