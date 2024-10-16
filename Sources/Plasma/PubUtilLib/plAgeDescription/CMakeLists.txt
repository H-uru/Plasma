set(plAgeDescription_SOURCES
    plAgeDescription.cpp
)

set(plAgeDescription_HEADERS
    plAgeDescription.h
    plAgeManifest.h
)

plasma_library(plAgeDescription
    FOLDER PubUtilLib
    SOURCES ${plAgeDescription_SOURCES} ${plAgeDescription_HEADERS}
)
target_link_libraries(
    plAgeDescription
    PUBLIC
        CoreLib
        plFile
        plUnifiedTime
    PRIVATE
        pnKeyedObject
)

source_group("Source Files" FILES ${plAgeDescription_SOURCES})
source_group("Header Files" FILES ${plAgeDescription_HEADERS})
