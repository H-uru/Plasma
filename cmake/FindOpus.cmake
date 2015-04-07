if(Opus_INCLUDE_DIR AND Opus_LIBRARY)
    set(Opus_FIND_QUIETLY TRUE)
endif()

find_path(Opus_INCLUDE_DIR opus.h
          /usr/local/include
          /usr/include
)

find_library(Opus_LIBRARY NAMES opus
             PATHS /usr/local/lib /usr/lib)

find_library(Celt_LIBRARY NAMES celt
             PATHS /usr/local/lib /usr/lib)

find_library(Silk_LIBRARY NAMES silk_common
             PATHS /usr/local/lib /usr/lib)

set(Opus_LIBRARIES
    ${Opus_LIBRARY}
    ${Celt_LIBRARY}
    ${Silk_LIBRARY}
)

if(Opus_INCLUDE_DIR AND Opus_LIBRARY)
    set(Opus_FOUND TRUE)
endif()

if (Opus_FOUND)
    if(NOT Opus_FIND_QUIETLY)
        message(STATUS "Found libopus: ${Opus_INCLUDE_DIR}")
    endif()
else()
    if(Opus_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find libopus")
    endif()
endif()
