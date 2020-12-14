set(plFileEncrypt_SOURCES
    main.cpp
)

add_executable(plFileEncrypt ${plFileEncrypt_SOURCES})
target_link_libraries(plFileEncrypt CoreLib plFile)
target_link_libraries(plFileEncrypt ${STRING_THEORY_LIBRARIES})

if(USE_VLD)
    target_link_libraries(plFileEncrypt ${VLD_LIBRARY})
endif()

install(
    TARGETS plFileEncrypt
    DESTINATION tools_cli
)

source_group("Source Files" FILES ${plFileEncrypt_SOURCES})
