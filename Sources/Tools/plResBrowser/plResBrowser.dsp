# Microsoft Developer Studio Project File - Name="plResBrowser" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=plResBrowser - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "plResBrowser.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "plResBrowser.mak" CFG="plResBrowser - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "plResBrowser - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "plResBrowser - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "plResBrowser - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../Sources/Plasma/NucleusLib/inc" /I "..\..\..\Sources\Plasma\PubUtilLib\inc" /I "..\..\..\Sources\Plasma\CoreLib" /I "../../../Sources/Plasma/FeatureLib/inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ws2_32.lib wininet.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib Shlwapi.lib libcrypto.a /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /libpath:"..\..\..\..\StaticSDKs\Win32\OpenSSL\lib"
# Begin Special Build Tool
TargetPath=.\Release\plResBrowser.exe
SOURCE="$(InputPath)"
PostBuild_Cmds=xcopy     /Y     $(TargetPath)      ..\..\..\tools\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "plResBrowser - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm- /GX /ZI /Od /I "../../../Sources/Plasma/NucleusLib/inc" /I "..\..\..\Sources\Plasma\PubUtilLib\inc" /I "..\..\..\Sources\Plasma\CoreLib" /I "../../../Sources/Plasma/FeatureLib/inc" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib wininet.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib Shlwapi.lib libcrypto.a /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc.lib" /pdbtype:sept /libpath:"..\..\..\..\StaticSDKs\Win32\OpenSSL\lib"
# Begin Special Build Tool
TargetPath=.\Debug\plResBrowser.exe
SOURCE="$(InputPath)"
PostBuild_Cmds=xcopy     /Y     $(TargetPath)      ..\..\..\tools\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "plResBrowser - Win32 Release"
# Name "plResBrowser - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\plPatchDetail.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Sources\Tools\plResBrowser\plResBrowser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Sources\Tools\plResBrowser\plResBrowserWndProc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Sources\Tools\plResBrowser\plResTreeView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Sources\Tools\plResBrowser\plWinRegistryTools.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\Sources\Tools\plResBrowser\plResTreeView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Sources\Tools\plResBrowser\plWinRegistryTools.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\dataicon.ico
# End Source File
# Begin Source File

SOURCE=..\..\..\Sources\Tools\plResBrowser\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\indexico.ico
# End Source File
# Begin Source File

SOURCE=.\res\mergedda.ico
# End Source File
# Begin Source File

SOURCE=.\res\mergedin.ico
# End Source File
# Begin Source File

SOURCE=..\..\..\Sources\Tools\plResBrowser\res\plResBrowser.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\Sources\Tools\plResBrowser\res\resource.h
# End Source File
# End Group
# End Target
# End Project
