if(Opus_INCLUDE_DIR AND Opus_LIBRARY)
    set(Opus_FIND_QUIETLY TRUE)
endif()

find_path(Opus_INCLUDE_DIR NAMES opus.h
          PATHS /usr/local/include /usr/include
          PATH_SUFFIXES opus)

find_library(Opus_LIBRARY NAMES opus
             PATHS /usr/local/lib /usr/lib)

find_library(Celt_LIBRARY NAMES celt
             PATHS /usr/local/lib /usr/lib)

find_library(Silk_LIBRARY NAMES silk_common
             PATHS /usr/local/lib /usr/lib)

if(Opus_INCLUDE_DIR AND Opus_LIBRARY)
    set(Opus_FOUND TRUE)
    if(Celt_LIBRARY AND Silk_LIBRARY)
        set(Opus_LIBRARIES
            ${Opus_LIBRARY}
            ${Celt_LIBRARY}
            ${Silk_LIBRARY}
        )
    else()
        set(Opus_LIBRARIES
            ${Opus_LIBRARY}
        )
    endif()
endif()

if(Opus_FOUND)
    if(NOT Opus_FIND_QUIETLY)
        message(STATUS "Found libopus: ${Opus_INCLUDE_DIR}")
    endif()
else()
    if(Opus_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find libopus")
    endif()
endif()
