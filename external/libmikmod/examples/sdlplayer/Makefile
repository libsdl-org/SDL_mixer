# Compiler, linker
CC=gcc
LD=$(CC)

# Full path to the libmikmod-config shell script.
LIBMIKMOD_CONFIG=libmikmod-config
LIBMIKMOD_CFLAGS=$(shell $(LIBMIKMOD_CONFIG) --cflags)
LIBMIKMOD_LIBS  =$(shell $(LIBMIKMOD_CONFIG) --libs)

# Full path to the sdl-config shell script.  E.g.:
# sdl-config for SDL-1.2, or sdl2-config for SDL2.
SDL_CONFIG=sdl-config
SDL_CFLAGS=$(shell $(SDL_CONFIG) --cflags)
SDL_LIBS  =$(shell $(SDL_CONFIG) --libs)

CFLAGS=-Wall -g $(LIBMIKMOD_CFLAGS) $(SDL_CFLAGS)
LDFLAGS=$(LIBMIKMOD_LIBS) $(SDL_LIBS)

PROG=sdlplay

OBJS=sdlplay.o

.PHONY : clean install

all: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(PROG)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) *.o $(PROG)
