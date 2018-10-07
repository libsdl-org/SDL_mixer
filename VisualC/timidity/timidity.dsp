# Microsoft Developer Studio Project File - Name="timidity" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=timidity - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "timidity.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "timidity.mak" CFG="timidity - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "timidity - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "timidity - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "timidity - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "timidity - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "timidity - Win32 Release"
# Name "timidity - Win32 Debug"
# Begin Source File

SOURCE=..\..\timidity\common.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\common.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\config.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\ctrlmode.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\ctrlmode.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\filter.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\filter.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\instrum.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\instrum.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\mix.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\mix.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\output.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\output.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\playmidi.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\playmidi.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\readmidi.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\readmidi.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\resample.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\resample.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\sdl_a.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\sdl_c.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\tables.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\tables.h
# End Source File
# Begin Source File

SOURCE=..\..\timidity\timidity.c
# End Source File
# Begin Source File

SOURCE=..\..\timidity\timidity.h
# End Source File
# End Target
# End Project
