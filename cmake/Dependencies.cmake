# List all dependency packages here.
set_package_properties(
    3dsm PROPERTIES
    URL "http://www.autodesk.com/"
    DESCRIPTION "SDK for integrating with the 3DS Max modelling software"
    PURPOSE "Required for building the Plasma plugins for 3DS Max"
    TYPE OPTIONAL
)
set_package_properties(
    ASIO PROPERTIES
    URL "https://think-async.com/Asio/"
    DESCRIPTION "Async framework for C++"
    TYPE REQUIRED
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
    expat PROPERTIES
    URL "https://libexpat.github.io/"
    DESCRIPTION "Expat XML Parser for C"
    TYPE REQUIRED
)
set_package_properties(
    freetype PROPERTIES
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
    LIBSECRET PROPERTIES
    URL "https://wiki.gnome.org/Projects/Libsecret"
    DESCRIPTION "Library for storing and retrieving passwords and other secrets"
    PURPOSE "Used to store credentials on many Unix-like platforms"
    TYPE RECOMMENDED
)
set_package_properties(
    libwebm PROPERTIES
    URL "https://github.com/webmproject/libwebm"
    DESCRIPTION "Library for parsing WebM video files"
    TYPE RECOMMENDED
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
set_package_properties(
    Qt5 PROPERTIES
    URL "http://www.qt.io/"
    DESCRIPTION "Cross-platform GUI framework for C++"
    PURPOSE "Required for building GUI tools for Plasma"
    TYPE OPTIONAL
)
set_package_properties(
    Security PROPERTIES
    URL "https://developer.apple.com/documentation/security"
    DESCRIPTION "Secure the data your app manages, and control access to your app"
    PURPOSE "Used to secure login credentials on Apple platforms"
    TYPE RECOMMENDED
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
    Uuid PROPERTIES
    DESCRIPTION "UUID library for Linux (provided by util-linux)"
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
