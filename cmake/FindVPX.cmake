if(VPX_INCLUDE_DIR AND VPX_LIBRARY)
    set(VPX_FIND_QUIETLY TRUE)
endif()

find_path(VPX_INCLUDE_DIR vpx/vp8.h
          /usr/local/include
          /usr/include
)

find_library(VPX_LIBRARY NAMES vpxmt vpxmd vpx
             PATHS /usr/local/lib /usr/lib
)

if(VPX_INCLUDE_DIR AND VPX_LIBRARY)
    set(VPX_FOUND TRUE)
endif()
