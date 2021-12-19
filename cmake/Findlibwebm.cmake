include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(libwebm_INCLUDE_DIR NAMES libwebm/mkvparser.hpp)
find_library(libwebm_LIBRARY_RELEASE NAMES webm libwebm)
find_library(libwebm_LIBRARY_DEBUG NAMES webmd libwebmd)

# Cope with vcpkg no longer renaming the debug library. This is needed due to MSVC's STL iterators
# not being ABI compatible from debug to release mode.
if(libwebm_LIBRARY_RELEASE AND NOT libwebm_LIBRARY_DEBUG)
    foreach(prefix_path IN LISTS CMAKE_PREFIX_PATH)
        if(prefix_path MATCHES "[Dd][Ee][Bb][Uu][Gg]\/?$")
            list(APPEND _debug_paths "${prefix_path}")
        endif()
        if(_debug_paths)
            find_library(libwebm_LIBRARY_DEBUG
                NAMES webm libwebm webmd libwembd
                PATHS ${_debug_paths}
                PATH_SUFFIXES lib
                NO_CMAKE_PATH
            )
        endif()
    endforeach()
endif()

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
