include_directories("../../Plasma/Apps")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/FeatureLib/inc")
include_directories("../../Plasma/FeatureLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories("../../Plasma/PubUtilLib")

set(plResBrowser_HEADERS
    plResBrowser.h
    plResTreeView.h
)
qt5_wrap_cpp(plResBrowser_MOC ${plResBrowser_HEADERS})

set(plResBrowser_SOURCES
    plResBrowser.cpp
    plResTreeView.cpp
)

if(WIN32)
    set(plResBrowser_HEADERS ${plResBrowser_HEADERS}
        plWinRegistryTools.h
    )

    set(plResBrowser_SOURCES ${plResBrowser_SOURCES}
        res/plResBrowser.rc
        plWinRegistryTools.cpp
    )
endif()

set(plResBrowser_RCC_SOURCES
    res/plResBrowser.qrc
)
qt5_add_resources(plResBrowser_RCC ${plResBrowser_RCC_SOURCES})

set(plResBrowser_UIC_SOURCES
    res/FindDialog.ui
    res/ResBrowser.ui
)
qt5_wrap_ui(plResBrowser_UIC ${plResBrowser_UIC_SOURCES})

# For generated ui_*.h files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(plResBrowser WIN32 MACOSX_BUNDLE
               ${plResBrowser_SOURCES} ${plResBrowser_HEADERS}
               ${plResBrowser_RCC} ${plResBrowser_UIC} ${plResBrowser_MOC})

target_link_libraries(plResBrowser CoreLib)
target_link_libraries(plResBrowser plResMgr)
target_link_libraries(plResBrowser pnSceneObject)
target_link_libraries(plResBrowser Qt5::Widgets)

if(USE_VLD)
    target_link_libraries(plResBrowser ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plResBrowser_SOURCES} ${plResBrowser_MOC})
source_group("Header Files" FILES ${plResBrowser_HEADERS})
source_group("Resource Files" FILES ${plResBrowser_RCC_SOURCES} ${plResBrowser_RCC}
                                    ${plResBrowser_UIC_SOURCES} ${plResBrowser_UIC})
