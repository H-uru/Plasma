set(plFileEncrypt_SOURCES
    main.cpp
)

plasma_executable(plFileEncrypt TOOL SOURCES ${plFileEncrypt_SOURCES})
target_link_libraries(
    plFileEncrypt
    PRIVATE
        CoreLib
        plFile
)

source_group("Source Files" FILES ${plFileEncrypt_SOURCES})
