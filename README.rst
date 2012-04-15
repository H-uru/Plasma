======
Plasma
======
A CMake-based fork of the GPLv3-licensed **CyanWorlds.com Engine** (Headspin/Plasma), with a focus on bug-fixes, cross-platform compatibility, and enhanced features.

For more information on Myst Online, see http://mystonline.com/developers/


Related projects
----------------
- moul-scripts  - https://github.com/H-uru/moul-scripts  - An updated repository of game scripts containing bug-fixes and compatible with modern python interpreters for use with Plasma.
- dirtsand  - https://github.com/H-uru/dirtsand  - An open-source Plasma-compatible server project.


Library Dependencies
--------------------
Plasma currently utilizes the following third-party libraries:

- nVidia PhysX 2.6.4  - http://developer.nvidia.com/physx-downloads
- Creative Labs' OpenAL SDK 1.1  - http://connect.creativelabs.com/openal/Downloads/OpenAL11CoreSDK.zip
- Microsoft DirectX SDK
  - November 2008  - http://www.microsoft.com/downloads/en/details.aspx?FamilyID=5493f76a-6d37-478d-ba17-28b1cca4865a or
  - June 2010  - http://www.microsoft.com/downloads/en/details.aspx?familyid=3021d52b-514e-41d3-ad02-438a3ba730ba


- Python 2.7  - http://www.python.org/download/releases/2.7/
- libOgg and libVorbis  - http://www.xiph.org/downloads/
- OpenSSL  - http://www.slproweb.com/products/Win32OpenSSL.html
- eXpat  - http://expat.sourceforge.net/
- libJPEG  - http://www.ijg.org/
- libPNG  - http://www.libpng.org/
- speex  - http://www.speex.org/downloads/
- zlib  - http://zlib.net/

- PyGTK  - http://www.pygtk.org/downloads.html
- PIL  - http://www.pythonware.com/products/pil/

Reducing the use of proprietary libraries is a focus of development and should be expected to change.

PhysX, OpenAL, and DirectX SDK will need to be acquired through the above links.
All other required libraries are available as precompiled binaries and associated files in the development libraries bundle (http://guildofwriters.com/tools/devlibs.zip) or can be built using their individual build instructions.


Compiling Instructions
----------------------
Currently, compilation only targets Windows systems and requires Visual Studio 2008 or Visual Studio 2010 (including Express Editions).

To compile:

#)  Start CMake-GUI
#)  Set the Where is the source code option to the location where you cloned the repository.
#)  Set the Where to build the binaries option to a subfolder of the aforementioned location called build.
#)  Check the Grouped and Advanced options.
#)  Press Configure. Select Visual Studio 9 2008 (or your preferred version of Visual Studio) as the generator.
#)  Set the CMAKE_INSTALL_PREFIX option under CMAKE to the cwe-prefix folder that you extracted from the development libraries bundle (http://guildofwriters.com/tools/devlibs.zip).
#)  Press Configure again.
#)  Set the OpenAL include and library path options under OpenAL.
    - **Default Include Path**: C:\\Program Files\\OpenAL 1.1 SDK\\include
    - **Default Library Path**: C:\\Program Files\\OpenAL 1.1 SDK\\lib\\win32\\OpenAL32.lib
#)  Press Configure again.
#) Set the PHYSX_SDK_PATH option under PHYSX. The default value is C:\\Program Files\\AGEIA Technologies\\AGEIA PhysX SDK\\v2.6.4\\SDKs.
#) Press Configure... For the last time!
#) Press Generate. You will now have a Visual Studio solution file (.sln) in the folder that you specified to build the binaries in.
#) Open the solution in Visual Studio. You can compile CyanWorlds.com Engine by pressing Build -> Build Solution. This will take some time. 


Running Instructions
--------------------

To run the Internal Client for testing with MOULa content, you will need the a fully-patched installation of MOULa provided by Cyan Worlds.  In addition, you will need to download or clone the files available on the moul-scripts repository (https://github.com/H-uru/moul-scripts).  If you followed the instructions above and built against Python 2.7, be sure to pull and use the python27 branch of moul-scripts.

#) Copy the files from your existing MOULa installation to a new folder, or install fresh if you do not already have it installed.  This folder will be referred to as MOUL-OS for the remainder of these instructions.
#) Copy the Python and SDL folders from moul-scripts into the MOUL-OS folder, as well as the files in dat into the existing dat folder.
#) Copy the example-server.ini file from the root of the Plasma repository into your MOUL-OS folder, and rename it as server.ini.  If you are running your own dirtsand server or are connecting to one run by someone else, use the server.ini generated from that.
#) Copy the DLLs from the development libraries bundle, as well as the DLLs PhysXLoader.dll, NxExtensions.dll, NxCooking.dll, and NxCharacter.dll from your PhysX SDK installation into the MOUL-OS folder.
#) Copy the resource.dat file from <build_dir>\\Sources\\Plasma\\Apps\\plClient\\external to the MOUL-OS folder, or from http://www.guildofwriters.com/tools/resource.dat if you did not build your own.
#) Create a shortcut in the MOUL-OS folder to the compiled plClient.exe.
#) Edit the shortcut's properties, and after the final quotation mark in the Target field, add /LocalData.  Also, change the Start in field to the path of your MOUL-OS folder.  
#) Double-click the shortcut to connect to your server and test!

Alternatively, if you wish to be able to debug using a single content folder from inside Visual Studio, you will need to do the following for each Configuration (Debug, Release, etc.) you have:

#) Open the Plasma Solution in Visual Studio.
#) Right-click on the plClient project in the Solution Explorer.
#) Select Configuration Properties->Debugging.
#) Enter /LocalData In the Command Arguments field.
#) Enter your MOUL-OS folder path in the Working Directory field.



Additional Information
----------------------
- Myst Online is available to play for free at http://mystonline.com/play/
- For more information on this fork and more in-depth building instructions, see the Guild of Writers wiki:  http://guildofwriters.com/wiki/Development:CyanWorlds.com_Engine.
- This code was forked from the initial release repository at OpenUru.org:  http://openuru.org/

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

OPENSSL
~~~~~~~
This product includes software developed by the OpenSSL Project for use in
the OpenSSL Toolkit (http://www.openssl.org/). This product includes
cryptographic software written by Eric A. Young (eay@cryptsoft.com). This
product includes software written by Tim J. Hudson (tjh@cryptsoft.com)."

Independent JPEG Group (IJG) JPEG Library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This software is based in part on the work of the Independent JPEG Group.

