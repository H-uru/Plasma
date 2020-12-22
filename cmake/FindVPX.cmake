include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

find_path(VPX_INCLUDE_DIR vpx/vp8.h
          /usr/local/include
          /usr/include
)

find_library(VPX_LIBRARY_RELEASE NAMES vpxmt vpxmd vpx
             PATHS /usr/local/lib /usr/lib
)
find_library(VPX_LIBRARY_DEBUG NAMES vpxmtd vpxmdd vpxd
             PATHS /usr/local/lib /usr/lib
)

select_library_configurations(VPX)
find_package_handle_standard_args(VPX REQUIRED_VARS VPX_INCLUDE_DIR VPX_LIBRARY)

if(VPX_FOUNS AND NOT TARGET VPX::VPX)
    add_library(VPX::VPX UNKNOWN IMPORTED)
    set_target_properties(
        VPX::VPX PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${VPX_INCLUDE_DIR}
    )
    if(EXISTS ${VPX_LIBRARY_DEBUG})
        set_target_properties(
            VPX::VPX PROPERTIES
            IMPORTED_LOCATION_DEBUG ${VPX_LIBRARY_DEBUG}
        )
    endif()
    if(EXISTS ${VPX_LIBRARY_RELEASE})
        set_target_properties(
            VPX::VPX PROPERTIES
            IMPORTED_LOCATION_RELEASE ${VPX_LIBRARY_RELEASE}
        )
    endif()
endif()

set(VPX_INCLUDE_DIRS ${VPX_INCLUDE_DIR})
