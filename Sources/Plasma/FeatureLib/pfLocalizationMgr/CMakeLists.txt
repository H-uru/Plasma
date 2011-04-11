include_directories(../../CoreLib)
include_directories(../../PubUtilLib)

include_directories(${EXPAT_INCLUDE_DIR})

add_definitions(-DXML_UNICODE_WCHAR_T)
add_definitions(-DXML_STATIC)
add_definitions(-DWIN32)

set(pfLocalizationMgr_SOURCES
    pfLocalizationDataMgr.cpp
    pfLocalizationMgr.cpp
    pfLocalizedString.cpp
)

set(pfLocalizationMgr_HEADERS
    pfLocalizationDataMgr.h
    pfLocalizationMgr.h
    pfLocalizedString.h
)

add_library(pfLocalizationMgr STATIC ${pfLocalizationMgr_SOURCES} ${pfLocalizationMgr_HEADERS})

source_group("Source Files" FILES ${pfLocalizationMgr_SOURCES})
source_group("Header Files" FILES ${pfLocalizationMgr_HEADERS})
