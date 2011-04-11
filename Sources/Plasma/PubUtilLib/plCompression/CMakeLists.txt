include_directories("../../CoreLib")

include_directories(${ZLIB_INCLUDE_DIR})

set(plCompression_SOURCES
    plZlibCompress.cpp
    plZlibStream.cpp
)

set(plCompression_HEADERS
    plCompress.h
    plZlibCompress.h
    plZlibStream.h
)

add_library(plCompression STATIC ${plCompression_SOURCES} ${plCompression_HEADERS})
target_link_libraries(plCompression ${ZLIB_LIBRARIES})

source_group("Source Files" FILES ${plCompression_SOURCES})
source_group("Header Files" FILES ${plCompression_HEADERS})
