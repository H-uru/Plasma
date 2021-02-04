set(plFileEncrypt_SOURCES
    main.cpp
)

add_executable(plFileEncrypt ${plFileEncrypt_SOURCES})
target_link_libraries(
    plFileEncrypt
    PRIVATE
        CoreLib
        plFile
)

install(
    TARGETS plFileEncrypt
    DESTINATION tools_cli
)

source_group("Source Files" FILES ${plFileEncrypt_SOURCES})
