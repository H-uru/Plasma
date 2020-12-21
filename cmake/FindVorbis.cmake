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

find_package_handle_standard_args(Vorbis
                                  REQUIRED_VARS Vorbis_INCLUDE_DIR
                                                Vorbis_LIBRARY VorbisFile_LIBRARY
)

set(Vorbis_INCLUDE_DIRS ${Vorbis_INCLUDE_DIR})
set(Vorbis_LIBRARIES ${Vorbis_LIBRARY} ${VorbisFile_LIBRARY})
