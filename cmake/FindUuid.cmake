include(FindPackageHandleStandardArgs)

find_path(Uuid_INCLUDE_DIR uuid/uuid.h
    /usr/local/include
    /usr/include
)

find_library(Uuid_LIBRARY NAMES uuid
    PATHS /usr/local/lib /usr/lib
)

find_package_handle_standard_args(Uuid REQUIRED_VARS Uuid_INCLUDE_DIR Uuid_LIBRARY)

if(Uuid_FOUND AND NOT TARGET Uuid::Uuid)
    add_library(Uuid::Uuid UNKNOWN IMPORTED)
    set_target_properties(
        Uuid::Uuid PROPERTIES
        IMPORTED_LOCATION ${Uuid_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${Uuid_INCLUDE_DIR}
    )
endif()
