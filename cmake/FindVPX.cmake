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

set(VPX_INCLUDE_DIRS ${VPX_INCLUDE_DIR})
