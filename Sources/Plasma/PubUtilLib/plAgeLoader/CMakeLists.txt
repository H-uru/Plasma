set(plAgeLoader_SOURCES
    plAgeLoader.cpp
    plAgeLoaderPaging.cpp
    plResPatcher.cpp
)

set(plAgeLoader_HEADERS
    plAgeLoader.h
    plAgeLoaderCreatable.h
    plResPatcher.h
)

plasma_library(plAgeLoader SOURCES ${plAgeLoader_SOURCES} ${plAgeLoader_HEADERS})

target_include_directories(plAgeLoader PRIVATE "${PLASMA_SOURCE_ROOT}/FeatureLib")
target_link_libraries(
    plAgeLoader
    PUBLIC
        CoreLib
        plAgeDescription
        pnKeyedObject
        pnNetBase
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
        pnSceneObject
        plFile
        plMessage
        plNetClient
        plProgressMgr
        plScene
        plSDL
        plResMgr
        pfPatcher # :(
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${plAgeLoader_SOURCES})
source_group("Header Files" FILES ${plAgeLoader_HEADERS})
