if(Speex_INCLUDE_DIR AND Speex_LIBRARY)
    set(Speex_FIND_QUIETLY TRUE)
endif()


find_path(Speex_INCLUDE_DIR speex.h
          /usr/local/include
          /usr/include
)

find_library(Speex_LIBRARY NAMES speex
             PATHS /usr/local/lib /usr/lib
)

set(Speex_LIBRARIES ${Speex_LIBRARY})


if(Speex_INCLUDE_DIR AND Speex_LIBRARY)
    set(Speex_FOUND TRUE)
endif()

if (Speex_FOUND)
    if(NOT Speex_FIND_QUIETLY)
        message(STATUS "Found speex: ${Speex_INCLUDE_DIR}")
    endif()
else()
    if(Speex_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find speex")
    endif()
endif()
