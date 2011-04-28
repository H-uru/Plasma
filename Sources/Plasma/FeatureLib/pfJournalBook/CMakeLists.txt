include_directories(../../CoreLib)
include_directories(../../FeatureLib)
include_directories(../../NucleusLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

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
