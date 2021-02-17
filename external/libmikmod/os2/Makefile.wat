# Makefile for OS/2 using Watcom compiler.
#
# wmake -f Makefile.wat
# - builds mikmod2.dll and its import lib mikmod2.lib
#
# wmake -f Makefile.wat target=static
# - builds the static library mikmod_static.lib

!ifndef target
target = dll
!endif

CPPFLAGS=-DMIKMOD_BUILD -DHAVE_FCNTL_H -DHAVE_LIMITS_H -DHAVE_MALLOC_H

# To build a debug version :
#CPPFLAGS+= -DMIKMOD_DEBUG

# MMPM/2 driver (will work with any OS/2 version starting from 2.1.)
CPPFLAGS+= -DDRV_OS2
# DART (Direct Audio Real Time) driver (uses less CPU time than the
#                          standard MMPM/2 drivers, requires Warp4.)
CPPFLAGS+= -DDRV_DART
# support for wav file output:
CPPFLAGS+= -DDRV_WAV
# support for output raw data to a file:
CPPFLAGS+= -DDRV_RAW
# support for output to stdout (not needed by everyone)
#CPPFLAGS+= -DDRV_STDOUT

# disable the high quality mixer (build only with the standart mixer)
#CPPFLAGS+= -DNO_HQMIXER

# drv_os2 and drv_dart require mmpm2
LIBS = mmpm2.lib

CFLAGS = -bt=os2 -bm -fp5 -fpi87 -mf -oeatxh -w4 -ei -zp8 -zq
# newer OpenWatcom versions enable W303 by default.
CFLAGS+= -wcd=303
# -5s  :  Pentium stack calling conventions.
# -5r  :  Pentium register calling conventions.
CFLAGS+= -5s
DLLFLAGS=-bd

.SUFFIXES:
.SUFFIXES: .obj .c

DLLNAME=mikmod2.dll
EXPNAME=mikmod2.exp
LIBNAME=mikmod2.lib
LIBSTATIC=mikmod_static.lib

!ifeq target static
CPPFLAGS+= -DMIKMOD_STATIC=1
BLD_TARGET=$(LIBSTATIC)
!else
CFLAGS+= $(DLLFLAGS)
BLD_TARGET=$(DLLNAME)
!endif

COMPILE=wcc386 $(CFLAGS) $(CPPFLAGS) $(INCLUDES)

OBJ=drv_os2.obj drv_dart.obj &
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
$(DLLNAME): $(OBJ)
	wlink NAM $@ OP q SYSTEM os2v2_dll INITINSTANCE TERMINSTANCE OPTION MANYAUTODATA LIBR {$(LIBS)} FIL {$(OBJ)} OPTION IMPF=$(EXPNAME)
	wlib -q -b -n -c -pa -s -t -zld -ii -io -inn $(LIBNAME) +$(DLLNAME)

$(LIBSTATIC): $(OBJ)
	wlib -q -b -n -c -pa -s -t -zld -ii -io $@ $(OBJ)

.c.obj:
	$(COMPILE) -fo=$^@ $<

!ifndef __UNIX__
distclean: clean .symbolic
	@if exist $(LIBSTATIC) del $(LIBSTATIC)
	@if exist $(DLLNAME) del $(DLLNAME)
	@if exist $(EXPNAME) del $(EXPNAME)
	@if exist $(LIBNAME) del $(LIBNAME)
clean: .symbolic
	@if exist *.obj del *.obj
.c: ..\drivers;..\loaders;..\mmio;..\playercode;..\posix
INCLUDES=-I..\os2 -I..\include
!else
distclean: clean .symbolic
	rm -f $(DLLNAME) $(EXPNAME) $(LIBNAME) $(LIBSTATIC)
clean: .symbolic
	rm -f *.obj
.c: ../drivers;../loaders;../mmio;../playercode;../posix
INCLUDES=-I../os2 -I../include
!endif
