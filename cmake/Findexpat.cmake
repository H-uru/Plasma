# Because cmake uses FindEXPAT -> EXPAT::EXPAT but upstream is expat::expat. Grrr...
set(_FIND_ARGS)
if(expat_FIND_QUIETLY)
    list(APPEND _FIND_ARGS QUIET)
endif()
if(expat_REQUIRED)
    list(APPEND _FIND_ARGS REQUIRED)
endif()

find_package(EXPAT ${_FIND_ARGS})
unset(_FIND_ARGS)

if(TARGET EXPAT::EXPAT AND NOT TARGET expat::expat)
    add_library(expat::expat ALIAS EXPAT::EXPAT)
endif()
