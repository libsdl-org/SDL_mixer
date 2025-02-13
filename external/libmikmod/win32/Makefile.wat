# Makefile for Win32 using Watcom compiler.
#
# wmake -f Makefile.wat
# - builds mikmod.dll and its import lib (mikmod.lib)
#
# wmake -f Makefile.wat target=static
# - builds the static library mikmod_static.lib
#
### NOTES ABOUT DRV_DS.C and OPEN WATCOM (ALSO SEE IN DRV_DS.C) :
#
# - If you encounter build failures from Open Watcom's dsound.h,
#   see: https://github.com/open-watcom/open-watcom-v2/pull/313
#
# - Open Watcom has broken __cdecl (leading underscore) name mangling for Windows
#   internal var names. It is fixed in Open Watcom V2 fork as of May/2014:
#   https://github.com/open-watcom/open-watcom-v2/commit/961ef1ff756f3ec5a7248cefcae00a6ecaa97ff4

!ifndef target
target = dll
!endif

INCLUDES=-I. -I"../include"
CPPFLAGS=-DMIKMOD_BUILD -DHAVE_FCNTL_H -DHAVE_LIMITS_H -DHAVE_MALLOC_H

# To build a debug version :
#CPPFLAGS+= -DMIKMOD_DEBUG

# support for WinMM sound output:
CPPFLAGS+=-DDRV_WIN
# support for Direct Sound output:
CPPFLAGS+=-DDRV_DS
# support for wav file output:
CPPFLAGS+= -DDRV_WAV
# support for output raw data to a file:
CPPFLAGS+= -DDRV_RAW
# support for output to stdout (not needed by everyone)
#CPPFLAGS+= -DDRV_STDOUT

# disable the high quality mixer (build only with the standart mixer)
#CPPFLAGS+= -DNO_HQMIXER

# for WinMM and Direct Sound output drivers
LIBS = winmm.lib dsound.lib dxguid.lib

CFLAGS = -bt=nt -bm -fp5 -fpi87 -mf -oeatxh -w4 -ei -zp8 -zq
# newer OpenWatcom versions enable W303 by default.
CFLAGS+= -wcd=303
# -5s  :  Pentium stack calling conventions.
# -5r  :  Pentium register calling conventions.
CFLAGS+= -5s
DLLFLAGS=-bd

.SUFFIXES:
.SUFFIXES: .obj .c

DLLNAME=libmikmod.dll
EXPNAME=libmikmod.exp
MAPNAME=libmikmod.map
LIBNAME=libmikmod.lib
LNKFILE=libmikmod.lnk
LIBSTATIC=mikmod-static.lib
LBCFILE=libmikmod.lbc

!ifeq target static
CPPFLAGS+= -DMIKMOD_STATIC=1
BLD_TARGET=$(LIBSTATIC)
!else
CPPFLAGS+= -DDLL_EXPORT=1
CFLAGS+= $(DLLFLAGS)
BLD_TARGET=$(DLLNAME)
!endif

COMPILE=wcc386 $(CFLAGS) $(CPPFLAGS) $(INCLUDES)

OBJ=drv_win.obj drv_ds.obj &
    drv_raw.obj drv_wav.obj &
    drv_nos.obj drv_stdout.obj &
    load_669.obj load_amf.obj load_dsm.obj load_far.obj load_gdm.obj &
    load_it.obj load_imf.obj load_m15.obj load_med.obj load_mod.obj load_mtm.obj load_okt.obj &
    load_s3m.obj load_stm.obj load_stx.obj load_ult.obj load_uni.obj load_xm.obj &
    mmalloc.obj mmerror.obj mmio.obj &
    strcasecmp.obj &
    mdriver.obj mdreg.obj mloader.obj mlreg.obj mlutil.obj mplayer.obj munitrk.obj mwav.obj &
    npertab.obj sloader.obj virtch.obj virtch2.obj virtch_common.obj

all: $(BLD_TARGET)

# rely on symbol name, not ordinal: -irn switch of wlib is default, but -inn is not.
$(DLLNAME): $(OBJ) $(LNKFILE)
	wlink @$(LNKFILE)
	wlib -q -b -n -c -pa -s -t -zld -ii -io -inn $(LIBNAME) +$(DLLNAME)

$(LIBSTATIC): $(OBJ) $(LBCFILE)
	wlib -q -b -n -c -pa -s -t -zld -ii -io $@ @$(LBCFILE)

.c: ../drivers;../loaders;../mmio;../playercode;../posix;
.c.obj:
	$(COMPILE) -fo=$^@ $<

distclean: clean .symbolic
	rm -f $(MAPNAME) $(LNKFILE) $(LBCFILE) 
	rm -f $(DLLNAME) $(EXPNAME) $(LIBNAME) $(LIBSTATIC)
clean: .symbolic
	rm -f *.obj

$(LNKFILE):
	@echo Creating linker file: $@
	@%create $@
	@%append $@ SYSTEM nt_dll INITINSTANCE TERMINSTANCE
	@%append $@ NAME $(DLLNAME)
	@for %i in ($(OBJ)) do @%append $@ FILE %i
	@for %i in ($(LIBS)) do @%append $@ LIB %i
	@%append $@ OPTION QUIET
	@%append $@ OPTION IMPF=$(EXPNAME)
	@%append $@ OPTION MAP=$(MAPNAME)
	@%append $@ OPTION SHOWDEAD

$(LBCFILE):
	@echo Creating wlib commands file: $@
	@%create $@
	@for %i in ($(OBJ)) do @%append $@ +%i
