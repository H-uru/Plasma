set(plPageOptimizer_SOURCES
    main.cpp
    plAllCreatables.cpp
    plPageOptimizer.cpp
)

set(plPageOptimizer_HEADERS
    plPageOptimizer.h
)

plasma_executable(plPageOptimizer TOOL
    FOLDER Tools
    SOURCES ${plPageOptimizer_SOURCES} ${plPageOptimizer_HEADERS}
)
target_link_libraries(
    plPageOptimizer
    PRIVATE
        CoreLib
        pnFactory
        pnKeyedObject
        pnNetCommon
        pnNucleusInc
        plGImage
        plMessage
        plPhysX
        plPubUtilInc
        plResMgr
        pfAnimation
        pfAudio
        pfCamera
        pfCharacter
        pfConditional
        pfGameGUIMgr
        pfGameMgr
        pfJournalBook
        pfMessage
        pfPython
        pfSurface
)

source_group("Source Files" FILES ${plPageOptimizer_SOURCES})
source_group("Header Files" FILES ${plPageOptimizer_HEADERS})
