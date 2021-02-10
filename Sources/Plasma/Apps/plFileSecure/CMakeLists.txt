set(plFileSecure_SOURCES
    main.cpp
)

add_executable(plFileSecure ${plFileSecure_SOURCES})
target_link_libraries(
    plFileSecure
    PRIVATE
        CoreLib
        plFile
)

install(
    TARGETS plFileSecure
    DESTINATION tools_cli
)

source_group("Source Files" FILES ${plFileSecure_SOURCES})
