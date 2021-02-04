if(LIBSECRET_FIND_REQUIRED)
    list(APPEND _FIND_ARGS REQUIRED)
endif()
if(LIBSECRET_FIND_QUIETLY)
    list(APPEND _FIND_ARGS QUIET)
endif()

find_package(PkgConfig ${_FIND_ARGS})

if(PkgConfig_FOUND)
    pkg_check_modules(LIBSECRET libsecret-1 IMPORTED_TARGET ${_FIND_ARGS})
endif()

unset(_FIND_ARGS)
