if(VPX_INCLUDE_DIR AND VPX_LIBRARY)
    set(VPX_FIND_QUIETLY TRUE)
endif()

find_path(VPX_INCLUDE_DIR vpx/vp8.h
          /usr/local/include
          /usr/include
)

find_library(VPX_LIBRARY NAMES vpxmt vpx
             PATHS /usr/local/lib /usr/lib
)

# If everything has been found, we have movie support!
if (VPX_INCLUDE_DIR AND VPX_LIBRARY)
    set(VPX_AVAILABLE TRUE)
    add_definitions(-DVPX_AVAILABLE)
endif()
