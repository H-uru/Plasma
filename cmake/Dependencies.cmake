# List all dependency packages here.
set_package_properties(
    3dsm PROPERTIES
    URL "http://www.autodesk.com/"
    DESCRIPTION "SDK for integrating with the 3DS Max modelling software"
    PURPOSE "Required for building the Plasma plugins for 3DS Max"
    TYPE OPTIONAL
)
set_package_properties(
    BZip2 PROPERTIES
    URL "https://www.sourceware.org/bzip2/"
    DESCRIPTION "Data compression library"
    PURPOSE "Required only if FreeType is built statically against bzip2"
    TYPE OPTIONAL
)
set_package_properties(
    CURL PROPERTIES
    URL "http://curl.haxx.se/libcurl/"
    DESCRIPTION "Multi-protocol file transfer library"
    TYPE REQUIRED
)
set_package_properties(
    DirectX PROPERTIES
    DESCRIPTION "Framework for hardware-accelerated 3D graphics on Microsoft platforms"
    TYPE REQUIRED
)
set_package_properties(
    EXPAT PROPERTIES
    URL "https://libexpat.github.io/"
    DESCRIPTION "Expat XML Parser for C"
    TYPE REQUIRED
)
set_package_properties(
    Freetype PROPERTIES
    URL "http://www.freetype.org/"
    DESCRIPTION "Library for rendering fonts"
    PURPOSE "Used to convert vector fonts to raster assets"
    TYPE OPTIONAL
)
set_package_properties(
    JPEG PROPERTIES
    URL "http://libjpeg-turbo.virtualgl.org/"
    DESCRIPTION "JPEG encoding and decoding library.  libjpeg-turbo is recommended for better performance"
    TYPE REQUIRED
)
set_package_properties(
    Ogg PROPERTIES
    URL "https://xiph.org/ogg/"
    DESCRIPTION "Ogg multimedia container library for various audio and video codecs"
    PURPOSE "Required for audio support in the client"
    TYPE REQUIRED
)
set_package_properties(
    OpenAL PROPERTIES
    URL "https://openal-soft.org/"
    DESCRIPTION "3D audio and EAX library"
    TYPE REQUIRED
)
set_package_properties(
    OpenSSL PROPERTIES
    URL "https://www.openssl.org/"
    DESCRIPTION "OpenSSL libraries for SSL/TLS"
    TYPE REQUIRED
)
set_package_properties(
    Opus PROPERTIES
    URL "https://www.opus-codec.org/"
    DESCRIPTION "Versatile free and open audio codec"
    PURPOSE "Required for in-game video playback and voice chat support"
    TYPE RECOMMENDED
)
set_package_properties(
    PhysX PROPERTIES
    URL "https://developer.nvidia.com/physx-sdk"
    DESCRIPTION "Scalable multi-platform physics solution"
    TYPE REQUIRED
)
set_package_properties(
    PNG PROPERTIES
    URL "http://www.libpng.org/pub/png/libpng.html"
    DESCRIPTION "Efficient lossless image library"
    TYPE REQUIRED
)
set_package_properties(
    Python3 PROPERTIES
    URL "http://www.python.org"
    DESCRIPTION "Python Scripting language C bindings"
    TYPE REQUIRED
)
set_package_properties(Qt5Core PROPERTIES
    URL "http://www.qt.io/"
    DESCRIPTION "Cross-platform GUI framework for C++"
    PURPOSE "Required for building GUI tools for Plasma"
    TYPE OPTIONAL
)
set_package_properties(
    Speex PROPERTIES
    URL "http://www.speex.org/"
    DESCRIPTION "Free and open audio codec specifically designed for speech"
    PURPOSE "Required for legacy in-game voice chat in the client"
    TYPE OPTIONAL
)
set_package_properties(
    string_theory PROPERTIES
    URL "http://github.com/zrax/string_theory"
    DESCRIPTION "Flexible C++11 string library and type-safe formatter"
    TYPE REQUIRED
)
set_package_properties(
    VLD PROPERTIES
    URL "https://kinddragon.github.io/vld/"
    DESCRIPTION "Visual Leak Debugger for software compiled with Microsoft Visual C++"
    PURPOSE "Useful for detecting memory leaks (MSVC only)"
    TYPE OPTIONAL
)
set_package_properties(
    Vorbis PROPERTIES
    URL "https://xiph.org/vorbis/"
    DESCRIPTION "Free and open audio codec for mid-to-high quality audio"
    PURPOSE "Required for audio support in the client"
    TYPE REQUIRED
)
set_package_properties(
    VPX PROPERTIES
    URL "http://www.webmproject.org/"
    DESCRIPTION "VP8 and VP9 video codec"
    PURPOSE "Required for in-game video playback support"
    TYPE RECOMMENDED
)
set_package_properties(
    ZLIB PROPERTIES
    URL "http://www.zlib.net"
    DESCRIPTION "Fast (de)compression library"
    TYPE REQUIRED
)

macro(plasma_find_dependency PACKAGE)
    cmake_parse_arguments(_pfd "" "FIXUP_TARGET" "" ${ARGN})

    find_package(${PACKAGE} ${_pfd_UNPARSED_ARGUMENTS})

    # Many old built-in find modules use the package name in all caps. Sad.
    # Eg. FindOpenAL -> OPENAL_FOUND, OPENAL_LIBRARY, etc.
    string(TOUPPER ${PACKAGE} _PACKAGE_UPPER)
    if(${UPPER_PREFIX}_FOUND)
        set(_FINDER_PREFIX ${UPPER_PREFIX})
    else()
        set(_FINDER_PREFIX ${INPUT_PREFIX})
    endif()

    # Note: this should only be used to help with built-in cmake find modules. If our own find
    # modules do not define a target, then they should be fixed to do so!
    if(${_FINDER_PREFIX}_FOUND AND DEFINED _pfd_FIXUP_TARGET AND NOT TARGET ${_pfd_FIXUP_TARGET})
        # If we have separate release and debug libraries, then the find module itself needs to
        # initialize the target. Targets don't support the select_library_configations format, and
        # it will be annoying to have to split out the imported library from the interface libraries.
        if(DEFINED ${_FINDER_PREFIX}_LIBRARY_RELEASE AND DEFINED ${_FINDER_PREFIX}_LIBRARY_DEBUG)
            message(FATAL_ERROR "Package '${PACKAGE}' uses library configurations but does not export a target!")
        endif()

        # Split the single library from the interfaces
        set(_INTERFACES ${_FINDER_PREFIX}_LIBRARIES)
        list(REMOVE_ITEM _INTERFACES ${_FINDER_PREFIXX}_LIBRARY)

        # Some old find modules don't follow the rules...
        if(${_FINDER_PREFIX}_INCLUDE_DIRS)
            set(_INCLUDE_DIRS ${_FINDER_PREFIX}_INCLUDE_DIRS)
        elseif(${_FINDER_PREFIX}_INCLUDE_DIR)
            set(_INCLUDE_DIRS ${_FINDER_PREFIX}_INCLUDE_DIR)
        else()
            message(FATAL_ERROR "Find${PACKAGE} does not seem to offer any include dirs?")
        endif()

        add_library(${_pfd_FIXUP_TARGET} STATIC IMPORTED)
        set_target_properties(
            ${_pfd_FIXUP_TARGET} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${_INCLUDE_DIRS}
            IMPORTED_LOCATION ${_FINDER_PREFIX}_LIBRARY
            INTERFACE_LINK_LIBRARIES ${_INTEFACES}
        )

        unset(_INTERFACES)
        unset(_INCLUDE_DIRS)
    endif()

    unset(_PACKAGE_UPPER)
    unset(_FINDER_PREFIX)
endmacro()
