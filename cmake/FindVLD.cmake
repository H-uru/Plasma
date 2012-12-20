if(VLD_INCLUDE_DIR AND VLD_LIBRARY)
    set(VLD_FIND_QUIETLY TRUE)
endif()


find_path(VLD_INCLUDE_DIR vld.h)
find_library(VLD_LIBRARY NAMES vld)
set(VLD_LIBRARIES ${VLD_LIBRARY})


if(VLD_INCLUDE_DIR AND VLD_LIBRARY)
    set(VLD_FOUND TRUE)
endif()

if (VLD_FOUND)
    if(NOT VLD_FIND_QUIETLY)
        message(STATUS "Found Visual Leak Detector: ${VLD_INCLUDE_DIR}")
    endif()
else()
    if(VLD_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find Visual Leak Detector")
    endif()
endif()
