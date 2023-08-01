Plasma
======
![CI](https://github.com/H-uru/Plasma/workflows/CI/badge.svg)

A CMake-based fork of the GPLv3-licensed **CyanWorlds.com Engine**
(Headspin/Plasma), with a focus on bug-fixes, cross-platform compatibility, and
enhanced features.

For more information on Myst Online, see http://mystonline.com/developers/


Related Projects
----------------

- [dirtsand](https://github.com/H-uru/dirtsand) - An open-source
  Plasma-compatible server project.
- [moul-assets](https://github.com/H-uru/moul-assets) - MOUL game
  assets repository.


Library Dependencies
--------------------

Plasma currently requires the following third-party libraries:

- NVIDIA PhysX 4.1 - https://github.com/NVIDIAGameWorks/PhysX
- Python 3.8 (or higher) - https://www.python.org/downloads/
- libOgg and libVorbis - http://www.xiph.org/downloads/
- OpenSSL - http://www.slproweb.com/products/Win32OpenSSL.html
- OpenAL Soft - https://openal-soft.org/
- eXpat - http://expat.sourceforge.net/
- Freetype - http://freetype.org/
- libJPEG-turbo - http://libjpeg-turbo.virtualgl.org/
- libPNG - http://www.libpng.org/
- zlib - http://zlib.net/
- libcurl - http://curl.haxx.se/
- string_theory - http://github.com/zrax/string_theory/
- Standalone ASIO - https://think-async.com/Asio/

The following libraries are optional:

- (for building resource.dat) CairoSVG - https://cairosvg.org/
- (for building resource.dat) Pillow - https://python-pillow.org/
- (for the GUI tools) Qt - http://www.qt.io/download-open-source/
- (for experimental OpenGL support) epoxy - https://github.com/anholt/libepoxy
- (for Linux font support) fontconfig - https://www.fontconfig.org/
- (for video) VPX - http://www.webmproject.org/
- (for video and voice chat) Opus - http://www.opus-codec.org/
- (for legacy voice chat) speex - http://www.speex.org/downloads/

All required libraries are available as [vcpkg](https://github.com/microsoft/vcpkg)
ports or can be built using their individual build instructions.


Compiling & Running Instructions
--------------------------------

The Plasma development site includes steps for
[compiling](https://h-uru.github.io/Plasma/building.html) and
[running](https://h-uru.github.io/Plasma/running.html) the client.

You can also download pre-built artifacts from our [last successful automated
build](https://github.com/H-uru/Plasma/releases/tag/last-successful) on
GitHub.

Additional Information
----------------------

- Myst Online is available to play for free at http://mystonline.com/play/
- This code was forked from the initial release repository at [OpenUru.org](http://openuru.org/).

**There are lots of opportunities to get involved!** Find out how you can [get
involved](https://h-uru.github.io/Plasma/involvement.html) and help improve Uru
for everyone.


About Non-Free Libraries
------------------------

This software uses some non-free libraries for which exceptions appear in the
source code license inserts. It is suggested that anyone who thinks of doing
substantial further work on this program should first free it from dependence
on the non-free libraries so that it does the same job without the non-free
libraries. Further introduction of non-free libraries likely would require a
revised license and thus permission from all contributors to the codebase.
That being problematic, any additional non-free libraries are unlikely to be
accepted by Cyan Worlds or the development community.


Acknowledgements
----------------

### OPENSSL ###
This product includes software developed by the OpenSSL Project for use in
the OpenSSL Toolkit (http://www.openssl.org/). This product includes
cryptographic software written by Eric A. Young (eay@cryptsoft.com). This
product includes software written by Tim J. Hudson (tjh@cryptsoft.com).

### Independent JPEG Group (IJG) JPEG Library ###
This software is based in part on the work of the Independent JPEG Group.
