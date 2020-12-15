include_directories(${PLASMA_SOURCE_ROOT}/CoreLib)
include_directories(${PLASMA_SOURCE_ROOT}/FeatureLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib/inc)
include_directories(${PLASMA_SOURCE_ROOT}/PubUtilLib)

set(pfJournalBook_SOURCES
    pfJournalBook.cpp
)

set(pfJournalBook_HEADERS
    pfJournalBook.h
    pfJournalBookCreatable.h
)

add_library(pfJournalBook STATIC ${pfJournalBook_SOURCES} ${pfJournalBook_HEADERS})

source_group("Source Files" FILES ${pfJournalBook_SOURCES})
source_group("Header Files" FILES ${pfJournalBook_HEADERS})
