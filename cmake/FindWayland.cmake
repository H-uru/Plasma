# Copyright (C) 2014, 2019 Igalia S.L.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#[==[.rst:
FindWayland
===========

Find Wayland libraries

COMPONENTS
----------

This module supports the following ``COMPONENTS``:

- ``Client``
- ``Server``
- ``EGL``
- ``Cursor``

If no components are specified, the ``Client`` component will be searched
for.

Imported Targets
----------------

This module defines the following :prop_tgt:`IMPORTED` targets, according
to the requested ``COMPONENTS``:

``Wayland::Client``
  The ``wayland-client`` library.
``Wayland::Server``
  The ``wayland-server`` library.
``Wayland::EGL``
  The ``wayland-egl`` library.
``Wayland::Cursor``
  The ``wayland-cursor`` library.

Result Variables
----------------

This module will set the following variables in your project:

``Wayland_FOUND``
  The requested Wayland components are available in the system.
``Wayland_VERSION``
  Version of the Wayland libraries (from ``wayland-version.h``).
``Wayland_<component>_FOUND``
  Whether the ``<component>`` is available in the system.
``Wayland_<component>_LIBRARY``
  Path to the library for the ``<component>``.
``Wayland_<component>_INCLUDE_DIR``
  The include directory for the ``<component>``.

Hints
-----

For each ``<component>``, the following may be set:

``Wayland_<component>_INCLUDE_DIR``
  Directory where to find headers.
``Wayland_<component>_LIBRARY``
  Path to the library.

#]==]

find_package(PkgConfig)

set(_Wayland_VALID_COMPONENTS Client Server Cursor EGL)

foreach (_comp IN LISTS _Wayland_VALID_COMPONENTS)
    set(Wayland_${_comp}_FOUND FALSE)
endforeach ()

if (NOT Wayland_FIND_COMPONENTS)
    # Search at least for the client library.
    list(APPEND Wayland_FIND_COMPONENTS Client)
endif ()

unset(Wayland_VERSION)
foreach (_comp IN LISTS Wayland_FIND_COMPONENTS)
    string(TOLOWER "${_comp}" _u_comp)

    if (NOT _comp IN_LIST _Wayland_VALID_COMPONENTS)
        message(WARNING "${_comp} is not a valid Wayland component")
        set(Wayland_${_comp}_FOUND FALSE)
        continue ()
    endif ()

    if (PkgConfig_FOUND)
        pkg_check_modules(Wayland_PC_${_comp} IMPORTED_TARGET wayland-${_u_comp})
        set(Wayland_${_comp}_VERSION "${Wayland_PC_${_comp}_VERSION}")
    endif ()

    if (NOT Wayland_VERSION)
        find_path(Wayland_VERSION_HEADER_PATH
            NAMES wayland-version.h
            HINTS ${Wayland_${_comp}_INCLUDE_DIRS})
        if (EXISTS "${Wayland_VERSION_HEADER_PATH}/wayland-version.h")
            file(STRINGS "${Wayland_VERSION_HEADER_PATH}/wayland-version.h"
                Wayland_VERSION REGEX
                "#[\t ]*define[\t ]+WAYLAND_VERSION[\t ]+\"[0-9\.]+\".*")
            string(REGEX MATCH "[0-9\.]+" Wayland_VERSION "${Wayland_VERSION}")
        endif ()
    endif ()

    if (NOT Wayland_${_comp}_VERSION AND Wayland_VERSION)
        set(Wayland_${_comp}_VERSION ${Wayland_VERSION})
    endif ()

    find_path(Wayland_${_comp}_INCLUDE_DIR
        NAMES wayland-client.h
        HINTS ${Wayland_${_comp}_INCLUDE_DIRS})
    find_library(Wayland_${_comp}_LIBRARY
        NAMES wayland-${_u_comp}
        PATHS ${Wayland_${_comp}_LIBRARY_DIRS})

    mark_as_advanced(Wayland_${_comp}_INCLUDE_DIR Wayland_${_comp}_LIBRARY)

    if (Wayland_${_comp}_LIBRARY AND NOT TARGET Wayland::${_comp})
        set(Wayland_${_comp}_FOUND TRUE)
        add_library(Wayland::${_comp} INTERFACE IMPORTED)
        if (TARGET PkgConfig::Wayland_PC_${_comp})
            set_property(TARGET Wayland::${_comp} PROPERTY
                INTERFACE_LINK_LIBRARIES PkgConfig::Wayland_PC_${_comp})
        else ()
            set_property(TARGET Wayland::${_comp} PROPERTY
                INTERFACE_LINK_LIBRARIES ${Wayland_${_comp}_LIBRARY})
            set_property(TARGET Wayland::${_comp} PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES ${Wayland_${_comp}_INCLUDE_DIR})
        endif ()
    endif ()
endforeach ()

if (Wayland_FIND_VERSION AND Wayland_VERSION AND
        Wayland_VERSION VERSION_LESS Wayland_FIND_VERSION)
    message(FATAL_ERROR "Package Wayland has version '${Wayland_VERSION}', "
                        "required version is '>= ${Wayland_FIND_VERSION}'")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Wayland REQUIRED_VARS Wayland_VERSION HANDLE_COMPONENTS)
