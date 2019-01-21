if(Vorbis_INCLUDE_DIR AND Vorbis_LIBRARY)
    set(Vorbis_FIND_QUIETLY TRUE)
endif()


find_path(Vorbis_INCLUDE_DIR vorbis/codec.h
          /usr/local/include
          /usr/include
)

find_library(Vorbis_LIBRARY
             NAMES vorbis libvorbis libvorbis_static
             PATHS /usr/local/lib /usr/lib
)

find_library(VorbisFile_LIBRARY
             NAMES vorbisfile libvorbisfile libvorbisfile_static
             PATHS /usr/local/lib /usr/lib
)

set(Vorbis_LIBRARIES
    ${Vorbis_LIBRARY}
    ${VorbisFile_LIBRARY}
)


if(Vorbis_INCLUDE_DIR AND Vorbis_LIBRARY AND VorbisFile_LIBRARY)
    set(Vorbis_FOUND TRUE)
endif()

if (Vorbis_FOUND)
    if(NOT Vorbis_FIND_QUIETLY)
        message(STATUS "Found libvorbis: ${Vorbis_INCLUDE_DIR}")
    endif()
else()
    if(Vorbis_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find libvorbis")
    endif()
endif()
