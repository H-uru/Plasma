set(pfFeatureInc_HEADERS
    pfAllCreatables.h
)
plasma_library(pfFeatureInc OBJECT SOURCES ${pfFeatureInc_HEADERS})
target_link_libraries(pfFeatureInc
    PUBLIC
        pfAnimation
        pfAudio
        pfCamera
        pfCCR
        pfCharacter
        pfConditional
        pfConsole
        pfGameGUIMgr
        pfJournalBook
        pfMessage
        pfPython
        pfSurface
)
target_include_directories(pfFeatureInc INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
