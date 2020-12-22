include(FindPackageHandleStandardArgs)

find_library(
    Security_LIBRARY
    NAMES Security
)

find_package_handle_standard_args(Security REQUIRED_VARS Security_LIBRARY)

if(Security_FOUND AND NOT TARGET Security::Security)
    add_library(Security::Security UNKNOWN IMPORTED)
    set_target_properties(
        Security::Security PROPERTIES
        IMPORTED_LOCATION ${Security_LIBRARY}
    )
endif()
