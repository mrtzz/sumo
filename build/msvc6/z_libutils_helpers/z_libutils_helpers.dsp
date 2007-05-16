# Microsoft Developer Studio Project File - Name="z_libutils_helpers" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=z_libutils_helpers - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "z_libutils_helpers.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "z_libutils_helpers.mak" CFG="z_libutils_helpers - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "z_libutils_helpers - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "z_libutils_helpers - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "z_libutils_helpers - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\src" /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "z_libutils_helpers - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\src" /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "z_libutils_helpers - Win32 Release"
# Name "z_libutils_helpers - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\Command.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\DiscreteCommand.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\gcc_NullType.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\gcc_Typelist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\gcc_TypeManip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\gcc_TypeTraits.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\msvc6_NullType.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\msvc6_static_check.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\msvc6_Typelist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\msvc6_TypeManip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\msvc6_TypeTraits.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\NamedObjectCont.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\OneArgumentCommand.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\RandomDistributor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\SUMODijkstraRouter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\ValueRetriever.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\utils\helpers\ValueSource.h
# End Source File
# End Group
# End Target
# End Project
