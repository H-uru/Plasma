set(pfMoviePlayer_SOURCES
    plMoviePlayer.cpp
    plPlanarImage.cpp
    $<$<BOOL:${USE_VIDEOTOOLBOX}>:plVTDecoder.cpp>
)

set(pfMoviePlayer_HEADERS
    plMoviePlayer.h
    plPlanarImage.h
    $<$<BOOL:${USE_VIDEOTOOLBOX}>:plVTDecoder.h>
)

plasma_library(pfMoviePlayer
    FOLDER FeatureLib
    SOURCES ${pfMoviePlayer_SOURCES} ${pfMoviePlayer_HEADERS}
)
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
        "$<$<BOOL:${USE_VIDEOTOOLBOX}>:-framework CoreMedia>"
        $<$<BOOL:${USE_VIDEOTOOLBOX}>:VideoToolbox::VideoToolbox>
)

source_group("Source Files" FILES ${pfMoviePlayer_SOURCES})
source_group("Header Files" FILES ${pfMoviePlayer_HEADERS})
