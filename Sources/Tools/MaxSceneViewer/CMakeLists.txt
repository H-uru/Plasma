include_directories("../")
include_directories("../../Plasma/Apps")
include_directories("../../Plasma/CoreLib")
include_directories("../../Plasma/FeatureLib")
include_directories("../../Plasma/FeatureLib/inc")
include_directories("../../Plasma/NucleusLib")
include_directories("../../Plasma/NucleusLib/inc")
include_directories("../../Plasma/PubUtilLib")
include_directories("../../Plasma/PubUtilLib/inc")
include_directories(${3dsm_INCLUDE_DIR})

set(MaxSceneViewer_HEADERS
    plKeyRefSort.h
    plMaxFileData.h
    plPluginApp.h
    plPluginClient.h
    SceneSync.h
    SceneViewer.h
    SceneWatcher.h
)

set(MaxSceneViewer_SOURCES
    plKeyRefSort.cpp
    plMaxFileData.cpp
    plPluginApp.cpp
    plPluginClient.cpp
    SceneSync.cpp
    SceneViewer.cpp
    SceneWatcher.cpp
)

add_library(MaxSceneViewer STATIC ${MaxSceneViewer_HEADERS} ${MaxSceneViewer_SOURCES})

source_group("Header Files" FILES ${MaxSceneViewer_HEADERS})
source_group("Source Files" FILES ${MaxSceneViewer_SOURCES})