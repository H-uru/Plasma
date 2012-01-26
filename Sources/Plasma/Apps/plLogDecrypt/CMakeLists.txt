include_directories("../../Apps")
include_directories("../../CoreLib")
include_directories("../../FeatureLib/inc")
include_directories("../../FeatureLib")
include_directories("../../NucleusLib/inc")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib/inc")
include_directories("../../PubUtilLib")

set(plLogDecrypt_SOURCES
    plLogDecrypt.cpp
)

add_executable(plLogDecrypt ${plLogDecrypt_SOURCES})
target_link_libraries(plLogDecrypt CoreLib plStatusLog pnProduct)

source_group("Source Files" FILES ${plLogDecrypt_SOURCES})
