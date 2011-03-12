# Microsoft Developer Studio Project File - Name="zlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=zlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak" CFG="zlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\zlibd.lib"

!ENDIF 

# Begin Target

# Name "zlib - Win32 Release"
# Name "zlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\src\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\src\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\src\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\src\gzio.c
# End Source File
# Begin Source File

SOURCE=..\..\src\infblock.c
# End Source File
# Begin Source File

SOURCE=..\..\src\infcodes.c
# End Source File
# Begin Source File

SOURCE=..\..\src\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\src\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\src\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\src\infutil.c
# End Source File
# Begin Source File

SOURCE=..\..\src\maketree.c
# End Source File
# Begin Source File

SOURCE=..\..\src\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\src\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\zutil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\deflate.h
# End Source File
# Begin Source File

SOURCE=..\..\src\infblock.h
# End Source File
# Begin Source File

SOURCE=..\..\src\infcodes.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\..\src\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\src\infutil.h
# End Source File
# Begin Source File

SOURCE=..\..\src\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\src\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\src\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\src\zutil.h
# End Source File
# End Group
# End Target
# End Project
