include(FindPackageHandleStandardArgs)

find_library(
    Security_LIBRARY
    NAMES Security
)

find_package_handle_standard_args(Security REQUIRED_VARS Security_LIBRARY)

if(Security_FOUND AND NOT TARGET Security::Security)
    add_library(Security::Security INTERFACE IMPORTED)
    set_target_properties(
        Security::Security PROPERTIES
        INTERFACE_LINK_LIBRARIES ${Security_LIBRARY}
    )
endif()
