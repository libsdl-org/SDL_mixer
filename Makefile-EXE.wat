#=============================================================================
#          This is a Watcom makefile to build SDLMIXER.DLL for OS/2
#
#
#=============================================================================

object_files= playwave.obj playmus.obj

# Extra stuffs to pass to C compiler:
ExtraCFlags=

#
#==============================================================================
#
!include Watcom-EXE.mif

.before
    @set include=$(%os2tk)\h;$(%include);$(%sdlhome)\include;.\;

all : playwave.exe playmus.exe

playwave.exe: $(object_files)
    wlink @playwave.lnk

playmus.exe: $(object_files)
    wlink @playmus.lnk

clean : .SYMBOLIC
    @if exist *.exe del *.exe
    @if exist *.obj del *.obj
    @if exist *.map del *.map
    @if exist *.res del *.res
    @if exist *.lst del *.lst
