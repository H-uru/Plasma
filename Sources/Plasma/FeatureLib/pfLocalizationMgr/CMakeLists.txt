include_directories(${PLASMA_SOURCE_ROOT}/CoreLib)
include_directories(${PLASMA_SOURCE_ROOT}/NucleusLib/inc)
include_directories(${PLASMA_SOURCE_ROOT}/PubUtilLib)

include_directories(${EXPAT_INCLUDE_DIR})

if(WIN32)
    add_definitions(-DWIN32)

    if(EXPAT_IS_STATIC)
        add_definitions(-DXML_STATIC)
    endif()
endif()

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
