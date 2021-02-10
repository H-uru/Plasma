# Because there are no CPP sources, this needs to be an interface library. Change to
# a static library if you ever add any real C++ code.
add_library(pfFeatureInc INTERFACE)
set_property(
    TARGET pfFeatureInc
    PROPERTY INTERFACE_LINK_LIBRARIES
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
set_property(
    TARGET pfFeatureInc
    PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}"
)
