set(plResBrowser_HEADERS
    plResBrowser.h
    plResTreeView.h
)

set(plResBrowser_SOURCES
    plResBrowser.cpp
    plResTreeView.cpp
)

if(WIN32)
    list(APPEND plResBrowser_HEADERS
        plWinRegistryTools.h
    )

    list(APPEND plResBrowser_SOURCES
        res/plResBrowser.rc
        plWinRegistryTools.cpp
    )
endif()

set(plResBrowser_RCC
    res/plResBrowser.qrc
)

set(plResBrowser_UIC
    res/FindDialog.ui
    res/ResBrowser.ui
)

add_executable(plResBrowser WIN32 MACOSX_BUNDLE
               ${plResBrowser_SOURCES} ${plResBrowser_HEADERS}
               ${plResBrowser_RCC} ${plResBrowser_UIC})
target_link_libraries(
    plResBrowser
    PRIVATE
        CoreLib
        pnFactory
        pnKeyedObject
        pnNucleusInc
        plResMgr
        Qt5::Widgets
)

set_target_properties(
    plResBrowser
    PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
)

target_include_directories(
    plResBrowser
    PRIVATE
        # Needed for custom widget include in ResBrowser.ui
        ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("Source Files" FILES ${plResBrowser_SOURCES} ${plResBrowser_MOC})
source_group("Header Files" FILES ${plResBrowser_HEADERS})
source_group("Resource Files" FILES ${plResBrowser_RCC} ${plResBrowser_UIC})
