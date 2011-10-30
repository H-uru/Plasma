include_directories("../../CoreLib")
include_directories("../../NucleusLib")
include_directories("../../PubUtilLib")

include_directories(${ZLIB_INCLUDE_DIR})

set(plFile_SOURCES
    hsFiles.cpp
    plBrowseFolder.cpp
    plEncryptedStream.cpp
    plFileUtils.cpp
    plInitFileReader.cpp
    plSecureStream.cpp
    plStreamSource.cpp
)

if(WIN32)
    set(plFile_SOURCES ${plFile_SOURCES} hsFiles_Win.cpp)
else(WIN32)
    set(plFile_SOURCES ${plFile_SOURCES} hsFiles_Unix.cpp)
endif(WIN32)

set(plFile_HEADERS
    hsFiles.h
    plBrowseFolder.h
    plEncryptedStream.h
    plFileUtils.h
    plInitFileReader.h
    plSecureStream.h
    plStreamSource.h
)

add_library(plFile STATIC ${plFile_SOURCES} ${plFile_HEADERS})
target_link_libraries(plFile CoreLib plUnifiedTime ${ZLIB_LIBRARIES})

source_group("Source Files" FILES ${plFile_SOURCES})
source_group("Header Files" FILES ${plFile_HEADERS})
