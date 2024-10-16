set(plMessageBox_SOURCES
    hsMessageBox.cpp
    $<$<PLATFORM_ID:Darwin>:hsMessageBox_Mac.mm>
    $<$<PLATFORM_ID:Windows>:hsMessageBox_Win.cpp>
)

set(plMessageBox_HEADERS
    hsMessageBox.h
)

plasma_library(plMessageBox
    FOLDER PubUtilLib
    SOURCES ${plMessageBox_SOURCES} ${plMessageBox_HEADERS}
)
target_link_libraries(
    plMessageBox
    PUBLIC
        CoreLib
    PRIVATE
        "$<$<PLATFORM_ID:Darwin>:-framework Cocoa>"
)

source_group("Source Files" FILES ${plMessageBox_SOURCES})
source_group("Header Files" FILES ${plMessageBox_HEADERS})
