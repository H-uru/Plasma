set(pfJournalBook_SOURCES
    pfJournalBook.cpp
)

set(pfJournalBook_HEADERS
    pfJournalBook.h
    pfJournalBookCreatable.h
)

plasma_library(pfJournalBook
    FOLDER FeatureLib
    SOURCES ${pfJournalBook_SOURCES} ${pfJournalBook_HEADERS}
)
target_link_libraries(
    pfJournalBook
    PUBLIC
        CoreLib
        pnKeyedObject
    PRIVATE
        pnMessage
        pnNetCommon
        pnNucleusInc
        plAgeLoader
        plGImage
        plMessage
        plResMgr
        plSurface
        pfGameGUIMgr
        pfMessage
        pfSurface
    INTERFACE
        pnFactory
)

source_group("Source Files" FILES ${pfJournalBook_SOURCES})
source_group("Header Files" FILES ${pfJournalBook_HEADERS})
