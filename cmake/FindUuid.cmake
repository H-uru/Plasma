if(Uuid_INCLUDE_DIR AND Uuid_LIBRARY)
    set(Uuid_FIND_QUIETLY TRUE)
endif()


find_path(Uuid_INCLUDE_DIR uuid/uuid.h
    /usr/local/include
    /usr/include
)

find_library(Uuid_LIBRARY NAMES uuid
    PATHS /usr/local/lib /usr/lib
)

set(Uuid_LIBRARIES ${Uuid_LIBRARY})


if(Uuid_INCLUDE_DIR AND Uuid_LIBRARY)
    set(Uuid_FOUND TRUE)
endif()

if(Uuid_FOUND)
    if(NOT Uuid_FIND_QUIETLY)
        message(STATUS "Found libuuid: ${Uuid_INCLUDE_DIR}")
    endif()
else()
    if(Uuid_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find libuuid")
    endif()
endif()
