include_directories("${PLASMA_SOURCE_ROOT}/Apps")
include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/FeatureLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib/inc")
include_directories("${PLASMA_SOURCE_ROOT}/PubUtilLib")

set(plFileEncrypt_SOURCES
    main.cpp
)

add_executable(plFileEncrypt ${plFileEncrypt_SOURCES})
target_link_libraries(plFileEncrypt CoreLib plFile)
target_link_libraries(plFileEncrypt ${STRING_THEORY_LIBRARIES})

if(USE_VLD)
    target_link_libraries(plFileEncrypt ${VLD_LIBRARY})
endif()

source_group("Source Files" FILES ${plFileEncrypt_SOURCES})
