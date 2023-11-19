include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(libarchive_INCLUDE_DIR
    NAMES archive.h
)
find_library(libarchive_LIBRARY_RELEASE NAMES archive libarchive)
find_library(libarchive_LIBRARY_DEBUG NAMES archive libarchive)

# Cope with vcpkg no longer renaming the debug library. This is needed due to MSVC's STL iterators
# not being ABI compatible from debug to release mode.
if(libwebm_LIBRARY_RELEASE AND NOT libwebm_LIBRARY_DEBUG)
    foreach(prefix_path IN LISTS CMAKE_PREFIX_PATH)
        if(prefix_path MATCHES "[Dd][Ee][Bb][Uu][Gg]\/?$")
            list(APPEND _debug_paths "${prefix_path}")
        endif()
        if(_debug_paths)
            find_library(libarchive_LIBRARY_DEBUG
                NAMES archive libarchive archived libarchived
                PATHS ${_debug_paths}
                PATH_SUFFIXES lib
                NO_CMAKE_PATH
            )
        endif()
    endforeach()
endif()

select_library_configurations(libarchive)
find_package_handle_standard_args(libarchive REQUIRED_VARS libarchive_INCLUDE_DIR libarchive_LIBRARY)

if(libarchive_FOUND AND NOT TARGET libarchive::libarchive)
    add_library(libarchive::libarchive IMPORTED UNKNOWN)
    set_target_properties(libarchive::libarchive PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${libarchive_INCLUDE_DIR})
    if(libarchive_LIBRARY_RELEASE AND libarchive_LIBRARY_DEBUG)
        set_target_properties(
            libarchive::libarchive PROPERTIES
            IMPORTED_LOCATION_DEBUG ${libarchive_LIBRARY_DEBUG}
            IMPORTED_LOCATION_RELEASE ${libarchive_LIBRARY_RELEASE}
        )
    else()
        set_target_properties(libarchive::libarchive PROPERTIES IMPORTED_LOCATION ${libarchive_LIBRARY})
    endif()
endif()
