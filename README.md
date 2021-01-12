Plasma
======

A CMake-based fork of the GPLv3-licensed **CyanWorlds.com Engine**
(Headspin/Plasma), with a focus on bug-fixes, cross-platform compatibility, and
enhanced features.

For more information on Myst Online, see http://mystonline.com/developers/

For a project roadmap, see https://github.com/H-uru/Plasma/wiki/Roadmap


Related Projects
----------------

- [dirtsand](https://github.com/H-uru/dirtsand) - An open-source
  Plasma-compatible server project.


Library Dependencies
--------------------

Plasma currently requires the following third-party libraries:

- Python 3.8 (or higher) - https://www.python.org/downloads/
- libOgg and libVorbis - http://www.xiph.org/downloads/
- OpenSSL - http://www.slproweb.com/products/Win32OpenSSL.html
- OpenAL Soft - https://openal-soft.org/
- eXpat - http://expat.sourceforge.net/
- libJPEG - http://libjpeg-turbo.virtualgl.org/
- libPNG - http://www.libpng.org/
- zlib - http://zlib.net/
- libcurl - http://curl.haxx.se/
- string_theory - http://github.com/zrax/string_theory/

The following libraries are optional:

- (for building resource.dat) CairoSVG - https://cairosvg.org/
- (for building resource.dat) Pillow - https://python-pillow.org/
- (for plFontConverter) Freetype - http://freetype.org/
- (for the GUI tools) Qt5 - http://www.qt.io/download-open-source/
- (for video) VPX - http://www.webmproject.org/
- (for video and voice chat) Opus - http://www.opus-codec.org/
- (for legacy voice chat) speex - http://www.speex.org/downloads/

All required libraries are available as [vcpkg](https://github.com/microsoft/vcpkg)
ports or can be built using their individual build instructions.


Compiling Instructions
----------------------

Currently, compilation only targets Windows systems and requires Visual Studio
2019 (including Visual Studio 2019 Community).

1. Clone the repository, including all submodules, in a git client or by
   executing the following at the command line:
   ```
   git clone --recurse-submodules https://github.com/H-uru/Plasma.git
   ```
2. Open **Microsoft Visual Studio 2019**.
3. Select **Open a local folder** and choose the folder where you cloned the
   repository.
4. Open the CMake Settings by chosing **Project > CMake Settings for Plasma**
   from the Visual Studio menu bar.
5. Add a new configuration in CMakeSettings.json by clicking the **green +**
   button.
6. Select **x86-Release** in the window that pops up.
7. Compile the client by using **Build > Install Plasma** from the Visual
   Studio menu bar.
8. The client will be built and installed into the *out\install\x86-Release*
   subfolder of where you cloned the repository. This will be referred to as
   the *MOUL-OS* folder.


Running Instructions
--------------------

To run the Internal Client for testing with MOULa content, you will need the a
fully-patched installation of MOULa provided by Cyan Worlds.

1. Copy the folders *avi*, *dat*, and *sfx* **from your existing MOULa installation**
   to the *MOUL-OS* folder.
2. Copy the example_server.ini file from the root of the Plasma repository into
   your *MOUL-OS* folder, and rename it as **server.ini**. If you are running
   your own dirtsand server or are connecting to one run by someone else, use
   the server.ini generated from that.
3. Create a **shortcut** in the *MOUL-OS* folder to the compiled plClient.exe.
4. Edit the shortcut's **properties**, and after the final quotation mark in the
   *Target* field, add `/LocalData`. Also, change the *Start in* field to the
   path of your *MOUL-OS* folder.
5. Double-click the shortcut to **connect** to your server and test!

Alternatively, if you wish to be able to debug using a single content folder
from inside Visual Studio:

1. Open the Plasma folder in Visual Studio.
2. Switch to *CMake Targets View* in the Solution Explorer.
3. Right-click on the *plClient (executable)* target in the Solution Explorer.
4. Select *Debug and Launch Settings > plClient.exe (Install)*.
5. Add the following to the *configurations*:
   ```json
   "args": "/LocalData"
   ```


Additional Information
----------------------

- Myst Online is available to play for free at http://mystonline.com/play/
- For more information on this fork and more in-depth building instructions,
  see the [Guild of Writers wiki](http://guildofwriters.org/wiki/Development:CyanWorlds.com_Engine).
- This code was forked from the initial release repository at [OpenUru.org](http://openuru.org/).


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
