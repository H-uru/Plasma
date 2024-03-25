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

plasma_library(plFile SOURCES ${plFile_SOURCES} ${plFile_HEADERS})
target_link_libraries(
    plFile
    PUBLIC
        CoreLib
    PRIVATE
        $<$<PLATFORM_ID:Windows>:shell32>
)

source_group("Source Files" FILES ${plFile_SOURCES})
source_group("Header Files" FILES ${plFile_HEADERS})
