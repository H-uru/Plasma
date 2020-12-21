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

set(Opus_INCLUDE_DIRS ${Opus_INCLUDE_DIR})
set(Opus_LIBRARIES ${Opus_LIBRARY})
if(Celt_LIBRARY)
    list(APPEND Opus_LIBRARIES ${Celt_LIBRARY})
endif()
if(Silk_LIBRARY)
    list(APPEND Opus_LIBRARIES ${Silk_LIBRARY})
endif()
