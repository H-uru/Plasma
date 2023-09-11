set(pfFeatureInc_HEADERS
    pfAllCreatables.h
)
plasma_library(pfFeatureInc OBJECT SOURCES ${pfFeatureInc_HEADERS})
target_link_libraries(pfFeatureInc
    PUBLIC
        pfAnimation
        pfAudio
        pfCamera
        pfCharacter
        pfConditional
        pfConsole
        pfGameGUIMgr
        pfGameMgr
        pfJournalBook
        pfMessage
        $<$<BOOL:${PLASMA_PIPELINE_DX}>:pfDXPipeline>
        $<$<BOOL:${PLASMA_PIPELINE_GL}>:pfGLPipeline>
        $<$<BOOL:${PLASMA_PIPELINE_METAL}>:pfMetalPipeline>
        pfPython
        pfSurface
)
target_include_directories(pfFeatureInc INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
