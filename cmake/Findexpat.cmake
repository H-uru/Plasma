# Because cmake uses FindEXPAT -> EXPAT::EXPAT but upstream is expat::expat. Grrr...
find_package(expat CONFIG QUIET)

if(NOT TARGET expat::expat)
    unset(CMAKE_MODULE_PATH)
    find_package(EXPAT MODULE)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

    if(EXPAT_FOUND)
        if(NOT TARGET expat::expat)
            add_library(expat::expat INTERFACE IMPORTED)
            target_link_libraries(expat::expat INTERFACE EXPAT::EXPAT)
        endif()
    elseif(expat_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find expat")
    endif()
endif()
