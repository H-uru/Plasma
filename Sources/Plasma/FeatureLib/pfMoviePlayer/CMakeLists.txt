include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

set(pfMoviePlayer_SOURCES
    plMoviePlayer.cpp
)

set(pfMoviePlayer_HEADERS
    plMoviePlayer.h
)

add_library(pfMoviePlayer STATIC ${pfMoviePlayer_SOURCES} ${pfMoviePlayer_HEADERS})

source_group("Source Files" FILES ${pfMoviePlayer_SOURCES})
source_group("Header Files" FILES ${pfMoviePlayer_HEADERS})
