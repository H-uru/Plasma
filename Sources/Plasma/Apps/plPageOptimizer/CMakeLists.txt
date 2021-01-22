set(plPageOptimizer_SOURCES
    main.cpp
    plAllCreatables.cpp
    plPageOptimizer.cpp
)

set(plPageOptimizer_HEADERS
    plPageOptimizer.h
)

add_executable(plPageOptimizer ${plPageOptimizer_SOURCES} ${plPageOptimizer_HEADERS})
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
        string_theory
)

install(
    TARGETS plPageOptimizer
    DESTINATION tools_cli
)

source_group("Source Files" FILES ${plPageOptimizer_SOURCES})
source_group("Header Files" FILES ${plPageOptimizer_HEADERS})
