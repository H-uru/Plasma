set(plFilePatcher_SOURCES
    plAllCreatables.cpp
    plFilePatcher.cpp
    main.cpp
)

set(plFilePatcher_HEADERS
    plFilePatcher.h
)

plasma_executable(plFilePatcher TOOL
    FOLDER Tools
    SOURCES ${plFilePatcher_SOURCES} ${plFilePatcher_HEADERS}
)
target_link_libraries(
    plFilePatcher
    PRIVATE
        CoreLib
        pnNetBase
        pnNucleusInc
        plNetGameLib
        pfConsoleCore
        pfPatcher
)

source_group("Source Files" FILES ${plFilePatcher_SOURCES})
source_group("Header Files" FILES ${plFilePatcher_HEADERS})
