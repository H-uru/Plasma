include(FindPackageHandleStandardArgs)

find_library(
    VideoToolbox_LIBRARY
    NAMES VideoToolbox
)

find_package_handle_standard_args(VideoToolbox REQUIRED_VARS VideoToolbox_LIBRARY)

if(VideoToolbox_FOUND AND NOT TARGET VideoToolbox::VideoToolbox)
    add_library(VideoToolbox::VideoToolbox INTERFACE IMPORTED)
    set_target_properties(
        VideoToolbox::VideoToolbox PROPERTIES
        INTERFACE_LINK_LIBRARIES ${VideoToolbox_LIBRARY}
    )
endif()

