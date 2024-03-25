set(plPageInfo_SOURCES
    plAllCreatables.cpp
    plPageInfo.cpp
)

plasma_executable(plPageInfo TOOL SOURCES ${plPageInfo_SOURCES})
target_link_libraries(
    plPageInfo
    PRIVATE
        CoreLib
        pnDispatch
        pnFactory
        pnKeyedObject
        pnMessage
        pnModifier
        pnNetCommon
        pnNucleusInc
        plAgeDescription
        plAudioCore
        plMessage
        plResMgr
)

source_group("Source Files" FILES ${plPageInfo_SOURCES})
