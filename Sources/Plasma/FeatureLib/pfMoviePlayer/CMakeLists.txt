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
target_link_libraries(
    pfMoviePlayer
    PUBLIC
        CoreLib
    PRIVATE
        plAudio
        plGImage
        plMessage
        plPipeline
        plResMgr
        plStatusLog
        $<$<BOOL:${USE_WEBM}>:Opus::opus>
        $<$<BOOL:${USE_WEBM}>:VPX::VPX>
)

source_group("Source Files" FILES ${pfMoviePlayer_SOURCES})
source_group("Header Files" FILES ${pfMoviePlayer_HEADERS})
source_group("WebM" FILES ${pfMoviePlayer_WEBM})
