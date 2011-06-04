include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

set(plEncryption_SOURCES
    plChecksum.cpp
)

set(plEncryption_HEADERS
    plChecksum.h
)

add_library(plEncryption STATIC ${plEncryption_SOURCES} ${plEncryption_HEADERS})

source_group("Source Files" FILES ${plEncryption_SOURCES})
source_group("Header Files" FILES ${plEncryption_HEADERS})
