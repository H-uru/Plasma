set(pnEncryption_SOURCES
    plBigNum.cpp
    plChallengeHash.cpp
    plChecksum.cpp
    plEncryption.cpp
    plSha0.cpp
)

set(pnEncryption_HEADERS
    plBigNum.h
    plChallengeHash.h
    plChecksum.h
    plEncryption.h
    plSha0.h
    plRandom.h
)

plasma_library(pnEncryption
    FOLDER NucleusLib
    SOURCES ${pnEncryption_SOURCES} ${pnEncryption_HEADERS}
)
target_link_libraries(
    pnEncryption
    PUBLIC
        CoreLib
        OpenSSL::Crypto
)

source_group("Source Files" FILES ${pnEncryption_SOURCES})
source_group("Header Files" FILES ${pnEncryption_HEADERS})
