set(plPageOptimizer_SOURCES
    main.cpp
    plAllCreatables.cpp
    plPageOptimizer.cpp
)

set(plPageOptimizer_HEADERS
    plPageOptimizer.h
)

plasma_executable(plPageOptimizer TOOL
    SOURCES ${plPageOptimizer_SOURCES} ${plPageOptimizer_HEADERS}
)
target_link_libraries(
    plPageOptimizer
    PRIVATE
        CoreLib
        pnDispatch
        pnFactory
        pnKeyedObject
        pnMessage
        pnModifier
        pnNetCommon
        pnNucleusInc
        plMessage
        plResMgr
)

source_group("Source Files" FILES ${plPageOptimizer_SOURCES})
source_group("Header Files" FILES ${plPageOptimizer_HEADERS})
