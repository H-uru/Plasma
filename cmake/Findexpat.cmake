# Because cmake uses FindEXPAT -> EXPAT::EXPAT but upstream is expat::expat. Grrr...
plasma_forward_find_package(expat CONFIG)
if(NOT TARGET expat::expat)
    plasma_forward_find_package(EXPAT MODULE ONLY_CMAKE_FIND_ROOT_PATH)

    if(EXPAT_FOUND)
        if(NOT TARGET expat::expat)
            add_library(expat::expat ALIAS EXPAT::EXPAT)
        endif()
    elseif(expat_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find expat")
    endif()
endif()
