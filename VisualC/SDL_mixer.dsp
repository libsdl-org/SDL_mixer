# Microsoft Developer Studio Project File - Name="SDL_mixer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SDL_mixer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SDL_mixer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SDL_mixer.mak" CFG="SDL_mixer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SDL_mixer - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SDL_mixer - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SDL_mixer - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\mikmod" /I "..\timidity" /I "..\native_midi" /I "fluidsynth\include" /I "vorbis\include" /I "smpeg\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WAV_MUSIC" /D "MOD_MUSIC" /D MOD_DYNAMIC=\"mikmod.dll\" /D "MID_MUSIC" /D "USE_TIMIDITY_MIDI" /D "USE_NATIVE_MIDI" /D "USE_FLUIDSYNTH_MIDI" /D FLUIDSYNTH_DYNAMIC=\"libfluidsynth.dll\" /D "OGG_MUSIC" /D OGG_DYNAMIC=\"libvorbisfile-3.dll\" /D "MP3_MUSIC" /D MP3_DYNAMIC=\"smpeg.dll\" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib SDL.lib /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "SDL_mixer - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gm /GX /Zi /Od /I "..\mikmod" /I "..\timidity" /I "..\native_midi" /I "fluidsynth\include" /I "vorbis\include" /I "smpeg\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WAV_MUSIC" /D "MOD_MUSIC" /D MOD_DYNAMIC=\"mikmod.dll\" /D MOD_DYNAMIC=\"mikmod.dll\" /D "MID_MUSIC" /D "USE_TIMIDITY_MIDI" /D "USE_NATIVE_MIDI" /D "USE_FLUIDSYNTH_MIDI" /D FLUIDSYNTH_DYNAMIC=\"libfluidsynth.dll\" /D "OGG_MUSIC" /D OGG_DYNAMIC=\"libvorbisfile-3.dll\" /D "MP3_MUSIC" /D MP3_DYNAMIC=\"smpeg.dll\" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib SDL.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "SDL_mixer - Win32 Release"
# Name "SDL_mixer - Win32 Debug"
# Begin Source File

SOURCE=..\dynamic_flac.c
# End Source File
# Begin Source File

SOURCE=..\dynamic_flac.h
# End Source File
# Begin Source File

SOURCE=..\dynamic_fluidsynth.c
# End Source File
# Begin Source File

SOURCE=..\dynamic_fluidsynth.h
# End Source File
# Begin Source File

SOURCE=..\dynamic_mod.c
# End Source File
# Begin Source File

SOURCE=..\dynamic_mod.h
# End Source File
# Begin Source File

SOURCE=..\dynamic_mp3.c
# End Source File
# Begin Source File

SOURCE=..\dynamic_mp3.h
# End Source File
# Begin Source File

SOURCE=..\dynamic_ogg.c
# End Source File
# Begin Source File

SOURCE=..\dynamic_ogg.h
# End Source File
# Begin Source File

SOURCE=..\effect_position.c
# End Source File
# Begin Source File

SOURCE=..\effect_stereoreverse.c
# End Source File
# Begin Source File

SOURCE=..\effects_internal.c
# End Source File
# Begin Source File

SOURCE=..\effects_internal.h
# End Source File
# Begin Source File

SOURCE=..\fluidsynth.c
# End Source File
# Begin Source File

SOURCE=..\fluidsynth.h
# End Source File
# Begin Source File

SOURCE=..\load_aiff.c
# End Source File
# Begin Source File

SOURCE=..\load_aiff.h
# End Source File
# Begin Source File

SOURCE=..\load_ogg.c
# End Source File
# Begin Source File

SOURCE=..\load_ogg.h
# End Source File
# Begin Source File

SOURCE=..\load_voc.c
# End Source File
# Begin Source File

SOURCE=..\load_voc.h
# End Source File
# Begin Source File

SOURCE=..\mixer.c
# End Source File
# Begin Source File

SOURCE=..\music.c
# End Source File
# Begin Source File

SOURCE=..\music_cmd.c
# End Source File
# Begin Source File

SOURCE=..\music_cmd.h
# End Source File
# Begin Source File

SOURCE=..\music_flac.c
# End Source File
# Begin Source File

SOURCE=..\music_flac.h
# End Source File
# Begin Source File

SOURCE=..\music_mad.c
# End Source File
# Begin Source File

SOURCE=..\music_mad.h
# End Source File
# Begin Source File

SOURCE=..\music_mod.c
# End Source File
# Begin Source File

SOURCE=..\music_mod.h
# End Source File
# Begin Source File

SOURCE=..\music_ogg.c
# End Source File
# Begin Source File

SOURCE=..\music_ogg.h
# End Source File
# Begin Source File

SOURCE=..\SDL_mixer.h
# End Source File
# Begin Source File

SOURCE=..\wavestream.c
# End Source File
# Begin Source File

SOURCE=..\wavestream.h
# End Source File
# Begin Source File

SOURCE=Version.rc
# End Source File
# End Target
# End Project
