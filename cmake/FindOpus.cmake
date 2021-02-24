find_package(Opus CONFIG QUIET)

if(NOT TARGET Opus::opus)
    include(FindPackageHandleStandardArgs)

    find_path(Opus_INCLUDE_DIR NAMES opus.h
            PATHS /usr/local/include /usr/include
            PATH_SUFFIXES opus)

    find_library(Opus_LIBRARY NAMES opus
                PATHS /usr/local/lib /usr/lib)

    find_library(Celt_LIBRARY NAMES celt
                PATHS /usr/local/lib /usr/lib)

    find_library(Silk_LIBRARY NAMES silk_common
                PATHS /usr/local/lib /usr/lib)

    find_package_handle_standard_args(Opus REQUIRED_VARS Opus_INCLUDE_DIR Opus_LIBRARY)

    if(Opus_FOUND AND NOT TARGET Opus::opus)
        add_library(Opus::opus UNKNOWN IMPORTED)
        set_target_properties(
            Opus::opus PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${Opus_INCLUDE_DIR}
            IMPORTED_LOCATION ${Opus_LIBRARY}
        )
        if(EXISTS ${Celt_LIBRARY})
            set_property(
                TARGET Opus::opus APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES ${Celt_LIBRARY}
            )
        endif()
        if(EXISTS ${Silk_LIBRARY})
            set_property(
                TARGET Opus::opus APPEND PROPERTY
                INTERFACE_LINK_LIBRARIES ${Silk_LIBRARY}
            )
        endif()
    endif()
endif()
