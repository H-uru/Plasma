include(FindPackageHandleStandardArgs)

find_path(Uuid_INCLUDE_DIR uuid/uuid.h
    /usr/local/include
    /usr/include
)

find_library(Uuid_LIBRARY NAMES uuid
    PATHS /usr/local/lib /usr/lib
)

find_package_handle_standard_args(Uuid REQUIRED_VARS Uuid_INCLUDE_DIR Uuid_LIBRARY)

set(Uuid_INCLUDE_DIRS ${Uuid_INCLUDE_DIRS})
set(Uuid_LIBRARIES ${Uuid_LIBRARY})
