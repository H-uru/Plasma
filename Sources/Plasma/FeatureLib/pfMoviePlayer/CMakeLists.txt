set(pfMoviePlayer_SOURCES
    plMoviePlayer.cpp
    plPlanarImage.cpp
)

set(pfMoviePlayer_HEADERS
    plMoviePlayer.h
    plPlanarImage.h
)

plasma_library(pfMoviePlayer SOURCES ${pfMoviePlayer_SOURCES} ${pfMoviePlayer_HEADERS})
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
        $<$<BOOL:${USE_WEBM}>:libwebm::libwebm>
        $<$<BOOL:${USE_WEBM}>:Opus::opus>
        $<$<BOOL:${USE_WEBM}>:VPX::VPX>
)

source_group("Source Files" FILES ${pfMoviePlayer_SOURCES})
source_group("Header Files" FILES ${pfMoviePlayer_HEADERS})
