if(Ogg_INCLUDE_DIR AND Ogg_LIBRARY)
    set(Ogg_FIND_QUIETLY TRUE)
endif()


find_path(Ogg_INCLUDE_DIR ogg/ogg.h
          /usr/local/include
          /usr/include
)

find_library(Ogg_LIBRARY NAMES ogg libogg libogg_static
             PATHS /usr/local/lib /usr/lib
)

set(Ogg_LIBRARIES ${Ogg_LIBRARY})


if(Ogg_INCLUDE_DIR AND Ogg_LIBRARY)
    set(Ogg_FOUND TRUE)
endif()

if (Ogg_FOUND)
    if(NOT Ogg_FIND_QUIETLY)
        message(STATUS "Found libogg: ${Ogg_INCLUDE_DIR}")
    endif()
else()
    if(Ogg_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find libogg")
    endif()
endif()
