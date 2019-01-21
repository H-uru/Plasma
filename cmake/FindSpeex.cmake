if(Speex_INCLUDE_DIR AND Speex_LIBRARY)
    set(Speex_FIND_QUIETLY TRUE)
endif()


#It would be a good idea to eventually use pkgconfig here.
find_path(Speex_INCLUDE_DIR speex/speex.h
          /usr/local/include
          /usr/include
)

find_library(Speex_LIBRARY NAMES speex libspeex
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
        if(NOT Speex_LIBRARIES)
          message(FATAL_ERROR "Could not find speex libraries")
        endif()
        if(NOT Speex_INCLUDE_DIR)
          message(FATAL_ERROR "Could not find speex includes")
        endif()
    endif()
endif()
