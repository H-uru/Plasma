include(FindPackageHandleStandardArgs)

find_path(Ogg_INCLUDE_DIR ogg/ogg.h
          PATHS /usr/local/include /usr/include
)

find_library(Ogg_LIBRARY
            NAMES ogg libogg libogg_static
            PATHS /usr/local/lib /usr/lib
)

find_package_handle_standard_args(Ogg REQUIRED_VARS Ogg_INCLUDE_DIR Ogg_LIBRARY)

set(Ogg_INCLUDE_DIRS ${Ogg_INCLUDE_DIR})
set(Ogg_LIBRARIES ${Ogg_LIBRARY})
