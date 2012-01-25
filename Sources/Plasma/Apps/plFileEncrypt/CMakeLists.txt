include_directories("../../Apps")
include_directories("../../CoreLib")
include_directories("../../FeatureLib/inc")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib/inc")
include_directories("../../PubUtilLib")

set(plFileEncrypt_SOURCES
    main.cpp
)

add_executable(plFileEncrypt ${plFileEncrypt_SOURCES})
target_link_libraries(plFileEncrypt CoreLib pnProduct plFile)

source_group("Source Files" FILES ${plFileEncrypt_SOURCES})
