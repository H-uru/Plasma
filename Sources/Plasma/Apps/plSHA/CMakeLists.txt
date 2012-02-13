include_directories("../../Apps")
include_directories("../../CoreLib")
include_directories("../../FeatureLib/inc")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib/inc")
include_directories("../../PubUtilLib")
include_directories(${OPENSSL_INCLUDE_DIR})

set(plSHA_SOURCES
    Main.cpp
)

add_executable(plSHA ${plSHA_SOURCES})
target_link_libraries(plSHA CoreLib pnUtils pnProduct pnEncryption)
target_link_libraries(plSHA ${OPENSSL_LIBRARIES})

source_group("Source Files" FILES ${plSHA_SOURCES})
