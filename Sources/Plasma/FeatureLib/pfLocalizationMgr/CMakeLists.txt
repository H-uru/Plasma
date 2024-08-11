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

plasma_library(pfLocalizationMgr
    FOLDER FeatureLib
    SOURCES ${pfLocalizationMgr_SOURCES} ${pfLocalizationMgr_HEADERS}
)

target_link_libraries(
    pfLocalizationMgr
    PUBLIC
        CoreLib
    PRIVATE
        plFile
        plResMgr
        plStatusLog
        expat::expat
)

source_group("Source Files" FILES ${pfLocalizationMgr_SOURCES})
source_group("Header Files" FILES ${pfLocalizationMgr_HEADERS})
