set(plFileSecure_SOURCES
    main.cpp
)

plasma_executable(plFileSecure TOOL SOURCES ${plFileSecure_SOURCES})
target_link_libraries(
    plFileSecure
    PRIVATE
        CoreLib
        plFile
)

source_group("Source Files" FILES ${plFileSecure_SOURCES})
