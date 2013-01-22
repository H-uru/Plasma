include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${ZLIB_INCLUDE_DIR})

set(plFile_SOURCES
    plBrowseFolder.cpp
    plEncryptedStream.cpp
    plInitFileReader.cpp
    plSecureStream.cpp
    plStreamSource.cpp
)

set(plFile_HEADERS
    plBrowseFolder.h
    plEncryptedStream.h
    plInitFileReader.h
    plSecureStream.h
    plStreamSource.h
)

add_library(plFile STATIC ${plFile_SOURCES} ${plFile_HEADERS})
target_link_libraries(plFile CoreLib plUnifiedTime ${ZLIB_LIBRARIES})

source_group("Source Files" FILES ${plFile_SOURCES})
source_group("Header Files" FILES ${plFile_HEADERS})
