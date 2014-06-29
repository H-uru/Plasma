Plasma
======

A CMake-based fork of the GPLv3-licensed **CyanWorlds.com Engine** (Headspin/Plasma), with a focus on bug-fixes, cross-platform compatibility, and enhanced features.

For more information on Myst Online, see http://mystonline.com/developers/

For a project roadmap, see https://github.com/H-uru/Plasma/wiki/Roadmap


Related Projects
----------------

- [moul-scripts](https://github.com/H-uru/moul-scripts) - An updated repository of game scripts containing bug-fixes and compatible with modern python interpreters for use with Plasma.
- [dirtsand](https://github.com/H-uru/dirtsand) - An open-source Plasma-compatible server project.


Library Dependencies
--------------------

Plasma currently requires the following third-party libraries:

- nVidia PhysX 2.6.4 - http://www.nvidia.com/object/physx_archives.html#SDK
- Microsoft DirectX SDK - http://www.microsoft.com/downloads/en/details.aspx?familyid=3021d52b-514e-41d3-ad02-438a3ba730ba
- Python 2.7 - http://www.python.org/download/releases/2.7/
- libOgg and libVorbis - http://www.xiph.org/downloads/
- OpenSSL - http://www.slproweb.com/products/Win32OpenSSL.html
- OpenAL Soft - http://kcat.strangesoft.net/openal.html
- eXpat - http://expat.sourceforge.net/
- libJPEG - http://libjpeg-turbo.virtualgl.org/
- libPNG - http://www.libpng.org/
- speex - http://www.speex.org/downloads/
- zlib - http://zlib.net/
- libcurl - http://curl.haxx.se/

The following libraries are optional:

- (for building resource.dat) PyGTK - http://www.pygtk.org/downloads.html
- (for building resource.dat) PIL - http://www.pythonware.com/products/pil/
- (for plFontConverter) Freetype - http://freetype.org/
- (for the GUI tools) Qt5 - http://qt-project.org/

Reducing the use of proprietary libraries is a focus of development and should be expected to change.

PhysX and DirectX SDK will need to be acquired through the above links.
All other required libraries are available as precompiled binaries and associated files in the [development libraries bundle](http://guildofwriters.org/tools/devlibs.zip) or can be built using their individual build instructions.


Compiling Instructions
----------------------

Currently, compilation only targets Windows systems and requires Visual Studio
2013 (including Visual Studio 2013 Express for Windows Desktop).

**Quick-start instructions:**

1. Run the `prepare_env.bat` script included in the repository.
2. You should now have a *build* folder with a Visual Studio solution file
   (.sln) inside.
3. Open the solution in Visual Studio. You can compile CyanWorlds.com Engine by
   pressing *Build -> Build Solution*. This will take some time.


**To configure manually with CMake and build:**

1. Start **CMake-GUI**.
2. Set the *Where is the source code* option to the location where you cloned
   the repository.
3. Set the *Where to build the binaries* option to a subfolder of the
   aforementioned location called *build*.
4. Check the **Grouped** and **Advanced** options.
5. Press **Configure**. Select *Visual Studio 12* as the generator.
6. Set the *CMAKE_INSTALL_PREFIX* option under CMAKE to the *cwe-prefix* folder
   that you extracted from the [development libraries
   bundle](http://guildofwriters.org/tools/devlibs.zip).
7. Press **Configure** again.
8. Press **Generate**. You will now have a Visual Studio solution file (.sln)
   in the folder that you specified to build the binaries in.
9. Open the solution in Visual Studio. You can compile CyanWorlds.com Engine by
   pressing *Build -> Build Solution*. This will take some time.


Running Instructions
--------------------

To run the Internal Client for testing with MOULa content, you will need the a fully-patched installation of MOULa provided by Cyan Worlds. In addition, you will need to download or clone the files available on the [moul-scripts repository](https://github.com/H-uru/moul-scripts).

1. Copy the **files from your existing MOULa installation** to a new folder, or install fresh if you do not already have it installed. This folder will be referred to as *MOUL-OS* for the remainder of these instructions.
2. Copy the **Python and SDL** folders from moul-scripts into the *MOUL-OS* folder, as well as the **files in dat** into the existing dat folder.
3. Copy the example_server.ini file from the root of the Plasma repository into your *MOUL-OS* folder, and rename it as **server.ini**. If you are running your own dirtsand server or are connecting to one run by someone else, use the server.ini generated from that.
4. Copy the **DLLs** from the development libraries bundle, as well as the DLLs PhysXLoader.dll, NxExtensions.dll, NxCooking.dll, and NxCharacter.dll from your PhysX SDK installation into the *MOUL-OS* folder.
5. Copy the **resource.dat** file from `<build_dir>\bin` to the *MOUL-OS* folder, or from http://www.guildofwriters.org/tools/resource.dat if you did not build your own.
6. Create a **shortcut** in the *MOUL-OS* folder to the compiled plClient.exe.
7. Edit the shortcut's **properties**, and after the final quotation mark in the *Target* field, add `/LocalData`. Also, change the *Start in* field to the path of your *MOUL-OS* folder.
8. Double-click the shortcut to **connect** to your server and test!

Alternatively, if you wish to be able to debug using a single content folder from inside Visual Studio, you will need to do the following for each Configuration (Debug, Release, etc.) you have:

1. Open the Plasma Solution in Visual Studio.
2. Right-click on the plClient project in the Solution Explorer.
3. Select *Configuration Properties->Debugging*.
4. Enter `/LocalData` in the *Command Arguments* field.
5. Enter your *MOUL-OS* folder path in the *Working Directory* field.


Additional Information
----------------------

- Myst Online is available to play for free at http://mystonline.com/play/
- For more information on this fork and more in-depth building instructions, see the [Guild of Writers wiki](http://guildofwriters.org/wiki/Development:CyanWorlds.com_Engine).
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
