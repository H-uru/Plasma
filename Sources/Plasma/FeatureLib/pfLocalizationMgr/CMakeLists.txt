include_directories(../../CoreLib)
include_directories(../../NucleusLib/inc)
include_directories(../../PubUtilLib)

# Skip CMake's built in FindExpat module and use libexpat's CMake config, if possible,
# due to library rename in libexpat 2.2.8 and higher.
find_package(EXPAT CONFIG QUIET)
if(NOT TARGET expat::libexpat)
    message(DEBUG "libexpat CMake Config not found, using FindExpat module.")
    find_package(EXPAT REQUIRED)
    add_library(expat::libexpat STATIC IMPORTED)
    set_target_properties(expat::libexpat PROPERTIES
                          INTERFACE_INCLUDE_DIRECTORIES ${EXPAT_INCLUDE_DIR}
                          IMPORTED_LOCATION ${EXPAT_LIBRARY}
    )
endif()

set_package_properties(EXPAT PROPERTIES
    URL "https://github.com/libexpat/libexpat"
    DESCRIPTION "Expat XML Parser for C"
    TYPE REQUIRED
)

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
target_link_libraries(pfLocalizationMgr expat::libexpat)

source_group("Source Files" FILES ${pfLocalizationMgr_SOURCES})
source_group("Header Files" FILES ${pfLocalizationMgr_HEADERS})
