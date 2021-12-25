include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(libwebm_INCLUDE_DIR
    NAMES mkvparser.hpp
    PATH_SUFFIXES libwebm
)
find_library(libwebm_LIBRARY_RELEASE NAMES webm libwebm)
find_library(libwebm_LIBRARY_DEBUG NAMES webmd libwebmd)

select_library_configurations(libwebm)
find_package_handle_standard_args(libwebm REQUIRED_VARS libwebm_INCLUDE_DIR libwebm_LIBRARY)

if(libwebm_FOUND AND NOT TARGET libwebm::libwebm)
    add_library(libwebm::libwebm IMPORTED UNKNOWN)
    set_target_properties(libwebm::libwebm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${libwebm_INCLUDE_DIR})
    if(libwebm_LIBRARY_RELEASE AND libwebm_LIBRARY_DEBUG)
        set_target_properties(
            libwebm::libwebm PROPERTIES
            IMPORTED_LOCATION_DEBUG ${libwebm_LIBRARY_DEBUG}
            IMPORTED_LOCATION_RELEASE ${libwebm_LIBRARY_RELEASE}
        )
    else()
        set_target_properties(libwebm::libwebm PROPERTIES IMPORTED_LOCATION ${libwebm_LIBRARY})
    endif()
endif()
