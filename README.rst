======
Plasma
======
A CMake-based fork of the GPLv3-licensed **CyanWorlds.com Engine** (Headspin/Plasma), with a focus on bug fixes, cross-platform compatibility, and enhanced features.

For more information, see http://mystonline.com/developers/


Related projects
----------------
- moul-scripts <https://github.com/H-uru/moul-scripts> - An updated repository of game scripts containing 
  bug-fixes and compatible with modern python interpreters for use with Plasma.
- dirtsand <https://github.com/H-uru/dirtsand> - An open-source Plasma-compatible server project.


Library Dependencies
--------------------
Plasma currently utilizes the following third-party libraries:

- Python 2.7 <http://www.python.org/download/releases/2.7/>
- nVidia PhysX 2.6.4 <http://developer.nvidia.com/physx-downloads>
- Creative Labs' OpenAL SDK 1.1 <http://connect.creativelabs.com/openal/Downloads/OpenAL11CoreSDK.zip>
- Microsoft DirectX SDK
  - November 2008 <http://www.microsoft.com/downloads/en/details.aspx?FamilyID=5493f76a-6d37-478d-ba17-28b1cca4865a> or
  - June 2010 <http://www.microsoft.com/downloads/en/details.aspx?familyid=3021d52b-514e-41d3-ad02-438a3ba730ba>
- libOgg and libVorbis <http://www.xiph.org/downloads/>
- OpenSSL <http://www.slproweb.com/products/Win32OpenSSL.html>
- eXpat <http://expat.sourceforge.net/>
- libJPEG <http://www.ijg.org/>
- speex <http://www.speex.org/downloads/>
- zlib <http://zlib.net/>

Reducing the use of proprietary libraries is a focus of development and should be expected to change.


Compiling Instructions
----------------------
Currently, compilation only targets Windows systems and requires Visual Studio 2008 or Visual Studio 2010.  

Support for Visual Studio Express editions and other operating systems is planned.

To compile:

1)  Start CMake-GUI
2)  Select your clone of the repository as the source directory, and a build directory (such as .\build)
3)  Click 'Configure'
    If CMake cannot find required libraries (such as PhsyX or OpenAL) you may need to specify them manually:
    Default locations are C:\\physx_2.6.4\\SDKs for **PHYSX_SDK_PATH**, while C:\\Program Files\\OpenAL 1.1 SDK\\include and C:\\Program Files\\OpenAL 1.1 SDK\\libs\\Win32\\OpenAL32.lib are **OPENAL_INCLUDE_DIR** and **OPENAL_LIBRARY**, respectively.  Once you have set the missing paths, click 'Configure' again to complete this step.
4)  Click 'Generate' to create the Visual Studio solution and associated projects.
5)  Open the `Plasma.sln` Solution in Visual Studio and choose **Build->Build Solution**.


Additional Information
----------------------
- Myst Online is available to play for free at http://mystonline.com/play/
- For more information on this fork and more in-depth building instructions, see the  Guild of Writers wiki: <http://guildofwriters.com/wiki/Development:CyanWorlds.com_Engine>.
- This code was forked from the initial release repository at  OpenUru.org <http://openuru.org/>