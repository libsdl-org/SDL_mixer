# Makefile.am for the SDL sample mixer library and players

lib_LTLIBRARIES = libSDL_mixer.la

SUBDIRS = @MUSIC_SUBDIRS@
DIST_SUBDIRS = mikmod timidity native_midi native_midi_gpl

libSDL_mixerincludedir = $(includedir)/SDL
libSDL_mixerinclude_HEADERS =	\
	SDL_mixer.h

libSDL_mixer_la_SOURCES =	\
	load_aiff.c		\
	load_aiff.h		\
	load_voc.c		\
	load_voc.h		\
	load_ogg.c		\
	load_ogg.h		\
	mixer.c			\
	music.c			\
	music_cmd.c		\
	music_cmd.h		\
	music_ogg.c		\
	music_ogg.h		\
	wavestream.c		\
	wavestream.h		\
	effect_position.c	\
	effect_stereoreverse.c	\
	effects_internal.c	\
	effects_internal.h

if USE_MIKMOD
MIKMOD_LIB = mikmod/libmikmod.la
else
MIKMOD_LIB =
endif
if USE_TIMIDITY
TIMIDITY_LIB = timidity/libtimidity.la
else
TIMIDITY_LIB =
endif
if USE_NATIVE_MIDI
if USE_NATIVE_MIDI_GPL
NATIVE_MIDI_LIB = native_midi_gpl/libnativemidi_gpl.la
else
NATIVE_MIDI_LIB = native_midi/libnativemidi.la
endif
else
NATIVE_MIDI_LIB =
endif

EXTRA_DIST =			\
	CHANGES			\
	VisualC.zip		\
	PBProjects.tar.gz	\
	MPWmake.sea.bin		\
	CWProjects.sea.bin	\
	SDL_mixer.spec		\
	SDL_mixer.spec.in	\
	autogen.sh

libSDL_mixer_la_LDFLAGS = 	\
	-no-undefined		\
	-release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-export-symbols-regex Mix_.*
libSDL_mixer_la_LIBADD = $(MIKMOD_LIB) $(TIMIDITY_LIB) $(NATIVE_MIDI_LIB) @SYSTEM_LIBS@ @SDL_LIBS@

noinst_PROGRAMS = playwave playmus

playwave_LDADD = libSDL_mixer.la @SDL_LIBS@
playmus_LDADD = libSDL_mixer.la @SDL_LIBS@

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: distcheck

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpm -ta $(PACKAGE)-$(VERSION).tar.gz

