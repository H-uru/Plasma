include_directories("${PLASMA_SOURCE_ROOT}/CoreLib")
include_directories("${PLASMA_SOURCE_ROOT}/NucleusLib")
include_directories(${OPENSSL_INCLUDE_DIR})

set(pnEncryption_SOURCES
    plBigNum.cpp
    plChallengeHash.cpp
    plChecksum.cpp
    plSha0.cpp
)

set(pnEncryption_HEADERS
    plBigNum.h
    plChallengeHash.h
    plChecksum.h
    plSha0.h
    plRandom.h
)

add_library(pnEncryption STATIC ${pnEncryption_SOURCES} ${pnEncryption_HEADERS})
target_link_libraries(pnEncryption CoreLib)
target_link_libraries(pnEncryption ${OPENSSL_LIBRARIES})

source_group("Source Files" FILES ${pnEncryption_SOURCES})
source_group("Header Files" FILES ${pnEncryption_HEADERS})
