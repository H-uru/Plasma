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

target_include_directories(pfLocalizationMgr PRIVATE ${EXPAT_INCLUDE_DIR})
target_link_libraries(pfLocalizationMgr
    PUBLIC
        ${EXPAT_LIBRARY}
        CoreLib
)

source_group("Source Files" FILES ${pfLocalizationMgr_SOURCES})
source_group("Header Files" FILES ${pfLocalizationMgr_HEADERS})
