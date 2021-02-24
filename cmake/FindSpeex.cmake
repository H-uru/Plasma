include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(Speex_INCLUDE_DIR speex/speex.h
          /usr/local/include
          /usr/include
)

find_library(Speex_LIBRARY_RELEASE NAMES speex libspeex
             PATHS /usr/local/lib /usr/lib
)
find_library(Speex_LIBRARY_DEBUG NAMES speexd libspeexd
             PATHS /usr/local/lib /usr/lib
)

select_library_configurations(Speex)
find_package_handle_standard_args(Speex REQUIRED_VARS Speex_INCLUDE_DIR Speex_LIBRARY)

if(Speex_FOUND AND NOT TARGET Speex::speex)
    add_library(Speex::speex UNKNOWN IMPORTED)
    set_target_properties(
        Speex::speex PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${Speex_INCLUDE_DIR}
    )
    if(EXISTS "${Speex_LIBRARY_DEBUG}" AND EXISTS "${Speex_LIBRARY_RELEASE}")
        set_target_properties(
            Speex::speex PROPERTIES
            IMPORTED_LOCATION_DEBUG ${Speex_LIBRARY_DEBUG}
            IMPORTED_LOCATION_RELEASE ${Speex_LIBRARY_RELEASE}
            MAP_IMPORTED_CONFIG_MINSIZEREL Release
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
        )
    else()
        set_target_properties(
            Speex::speex PROPERTIES
            IMPORTED_LOCATION ${Speex_LIBRARY}
        )
    endif()
endif()
