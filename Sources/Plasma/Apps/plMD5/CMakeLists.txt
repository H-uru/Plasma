include_directories("../../Apps")
include_directories("../../CoreLib")
include_directories("../../FeatureLib/inc")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib/inc")
include_directories("../../PubUtilLib")
include_directories(${OPENSSL_INCLUDE_DIR})

set(plMD5_SOURCES
    Main.cpp
)

add_executable(plMD5 ${plMD5_SOURCES})
target_link_libraries(plMD5 CoreLib pnUtils pnProduct pnEncryption)
target_link_libraries(plMD5 ${OPENSSL_LIBRARIES})

source_group("Source Files" FILES ${plMD5_SOURCES})
