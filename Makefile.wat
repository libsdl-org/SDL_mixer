#=============================================================================
#          This is a Watcom makefile to build SDLMIXER.DLL for OS/2
#
#
#=============================================================================

dllname=SDLMIXER

mainobjs = effects_internal.obj effect_position.obj effect_stereoreverse.obj load_aiff.obj load_ogg.obj load_voc.obj mixer.obj music.obj music_cmd.obj music_ogg.obj wavestream.obj
mikmodobjs = mikmod\drv_nos.obj mikmod\drv_sdl.obj mikmod\load_it.obj mikmod\load_mod.obj mikmod\load_s3m.obj mikmod\load_xm.obj mikmod\mdreg.obj mikmod\mdriver.obj mikmod\mloader.obj mikmod\mlreg.obj mikmod\mlutil.obj mikmod\mmalloc.obj mikmod\mmerror.obj mikmod\mmio.obj mikmod\mplayer.obj mikmod\munitrk.obj mikmod\mwav.obj mikmod\npertab.obj mikmod\sloader.obj mikmod\virtch.obj mikmod\virtch2.obj mikmod\virtch_common.obj
timidityobjs = timidity\common.obj timidity\controls.obj timidity\filter.obj timidity\instrum.obj timidity\mix.obj timidity\output.obj timidity\playmidi.obj timidity\readmidi.obj timidity\resample.obj timidity\sdl_a.obj timidity\sdl_c.obj timidity\tables.obj timidity\timidity.obj
oggidecobjs = oggidec\bitwise.obj oggidec\block.obj oggidec\codebook.obj oggidec\floor0.obj oggidec\floor1.obj oggidec\framing.obj oggidec\info.obj oggidec\mapping0.obj oggidec\mdct.obj oggidec\registry.obj oggidec\res012.obj oggidec\sharedbook.obj oggidec\synthesis.obj oggidec\vorbisfile.obj oggidec\window.obj
oggobjs = libogg-1.0\src\bitwise.obj libogg-1.0\src\framing.obj
vorbisobjs = libvorbis-1.0\lib\analysis.obj &
             libvorbis-1.0\lib\bitrate.obj &
             libvorbis-1.0\lib\block.obj &
             libvorbis-1.0\lib\codebook.obj &
             libvorbis-1.0\lib\envelope.obj &
             libvorbis-1.0\lib\floor0.obj &
             libvorbis-1.0\lib\floor1.obj &
             libvorbis-1.0\lib\info.obj &
             libvorbis-1.0\lib\lookup.obj &
             libvorbis-1.0\lib\lpc.obj &
             libvorbis-1.0\lib\lsp.obj &
             libvorbis-1.0\lib\mapping0.obj &
             libvorbis-1.0\lib\mdct.obj &
             libvorbis-1.0\lib\psy.obj &
             libvorbis-1.0\lib\registry.obj &
             libvorbis-1.0\lib\res0.obj &
             libvorbis-1.0\lib\sharedbook.obj &
             libvorbis-1.0\lib\smallft.obj &
             libvorbis-1.0\lib\synthesis.obj &
             libvorbis-1.0\lib\vorbisenc.obj &
             libvorbis-1.0\lib\vorbisfile.obj &
             libvorbis-1.0\lib\window.obj


object_files= $(mainobjs) $(mikmodobjs) $(timidityobjs) $(oggidecobjs)

# Extra stuffs to pass to C compiler:
ExtraCFlags=

#
#==============================================================================
#
!include Watcom.mif

.before
    @set include=$(%os2tk)\h;$(%include);$(%sdlhome)\include;.\timidity;.\mikmod;.\oggidec;
    @cd mikmod
    @wmake -h -f Makefile.wat
    @cd ..\timidity
    @wmake -h -f Makefile.wat
    @cd ..

all : $(dllname).dll $(dllname).lib

$(dllname).dll : $(object_files)
    wlink @$(dllname)

$(dllname).lib : $(dllname).dll
    implib $(dllname).lib $(dllname).dll

clean : .SYMBOLIC
    @if exist *.dll del *.dll
    @if exist *.lib del *.lib
    @if exist *.obj del *.obj
    @if exist *.map del *.map
    @if exist *.res del *.res
    @if exist *.lst del *.lst
    @cd mikmod
    @wmake -h -f Makefile.wat clean
    @cd ..\timidity
    @wmake -h -f Makefile.wat clean
    @cd ..
