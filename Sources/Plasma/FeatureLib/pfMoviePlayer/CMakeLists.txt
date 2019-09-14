include_directories(../../CoreLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

if(PLASMA_USE_WEBM)
    include_directories(${VPX_INCLUDE_DIR})
    include_directories(${Opus_INCLUDE_DIR})
endif()

set(pfMoviePlayer_SOURCES
    plMoviePlayer.cpp
    plPlanarImage.cpp
)

set(pfMoviePlayer_HEADERS
    plMoviePlayer.h
    plPlanarImage.h
)

set(pfMoviePlayer_WEBM
    webm/mkvparser.cpp
    webm/mkvparser.hpp
    webm/mkvreader.cpp
    webm/mkvreader.hpp
)

add_library(pfMoviePlayer STATIC ${pfMoviePlayer_SOURCES} ${pfMoviePlayer_HEADERS} ${pfMoviePlayer_WEBM})

source_group("Source Files" FILES ${pfMoviePlayer_SOURCES})
source_group("Header Files" FILES ${pfMoviePlayer_HEADERS})
source_group("WebM" FILES ${pfMoviePlayer_WEBM})
