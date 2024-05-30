include(FindPackageHandleStandardArgs)

find_library(
    Metal_LIBRARY
    NAMES Metal
)

find_package_handle_standard_args(Metal REQUIRED_VARS Metal_LIBRARY)

if(Metal_FOUND AND NOT TARGET Metal::Metal)
    add_library(Metal::Metal INTERFACE IMPORTED)
    set_target_properties(
        Metal::Metal PROPERTIES
        INTERFACE_LINK_LIBRARIES ${Metal_LIBRARY}
    )
endif()

