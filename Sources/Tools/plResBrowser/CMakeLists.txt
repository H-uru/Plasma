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

plasma_executable(plResBrowser TOOL QT_GUI
    SOURCES
        ${plResBrowser_SOURCES} ${plResBrowser_HEADERS}
        ${plResBrowser_RCC} ${plResBrowser_UIC}
)
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

source_group("Source Files" FILES ${plResBrowser_SOURCES} ${plResBrowser_MOC})
source_group("Header Files" FILES ${plResBrowser_HEADERS})
source_group("Resource Files" FILES ${plResBrowser_RCC} ${plResBrowser_UIC})
