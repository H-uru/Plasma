find_package(Vorbis CONFIG QUIET)

if(NOT TARGET Vorbis::vorbis)
    include(FindPackageHandleStandardArgs)

    find_path(Vorbis_INCLUDE_DIR vorbis/codec.h
            PATHS /usr/local/include /usr/include
    )

    find_library(Vorbis_LIBRARY
                NAMES vorbis libvorbis libvorbis_static
                PATHS /usr/local/lib /usr/lib
    )

    find_library(VorbisFile_LIBRARY
                NAMES vorbisfile libvorbisfile libvorbisfile_static
                PATHS /usr/local/lib /usr/lib
    )

    find_package_handle_standard_args(
        Vorbis REQUIRED_VARS Vorbis_INCLUDE_DIR Vorbis_LIBRARY VorbisFile_LIBRARY
    )

    if(Vorbis_FOUND AND NOT TARGET Vorbis::vorbis)
        add_library(Vorbis::vorbis UNKNOWN IMPORTED)
        set_target_properties(
            Vorbis::vorbis PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${Vorbis_INCLUDE_DIR}
            IMPORTED_LOCATION ${Vorbis_LIBRARY}
        )
    endif()

    if(Vorbis_FOUND AND NOT TARGET Vorbis::vorbisfile)
        add_library(Vorbis::vorbisfile UNKNOWN IMPORTED)
        set_target_properties(
            Vorbis::vorbisfile PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${Vorbis_INCLUDE_DIR}
            IMPORTED_LOCATION ${VorbisFile_LIBRARY}
        )
    endif()
endif()
