#CFLAGS := -c -flto -O2 -Wall $(shell pkg-config --cflags sdl2 SDL2_image SDL2_mixer SDL2_gfx)
#CFLAGS=-c -O2 -Wall -g -pg
CFLAGS=-c -flto -O2 -Wall


LIBS := $(shell pkg-config --libs sdl SDL_image SDL_mixer SDL_gfx ogg vorbis vorbisfile)

LDFLAGS=

PCLDFLAGS=-L/usr/lib -Wl,-rpath,/opt/usr/lib \
		-lSDL_image -lSDL_gfx -ljpeg -lpng16 -lz -lSDL_mixer -lvorbisidec -lvorbisfile -lvorbis -lSDL \
		-logg -lpthread -lgcc -lm -lc

SOURCES=cmdline.c encoding.c err.c fileio.c help.c lxlogic.c mslogic.c play.c random.c res.c \
		score.c series.c solution.c tworld.c unslist.c \
		oshw-sdl/SFont.c oshw-sdl/ccicon.c oshw-sdl/sdlerr.c oshw-sdl/sdlin.c \
		oshw-sdl/sdloshw.c oshw-sdl/sdlout.c oshw-sdl/sdlsfx.c oshw-sdl/sdltext.c \
		oshw-sdl/sdltile.c oshw-sdl/sdltimer.c oshw-sdl/port_cfg.c


OBJECTS=$(SOURCES:.c=.o)
EXECUTABLEPC=tworld

CC=gcc
CFLAGS+=-I/usr/include \
		-I/usr/include/SDL \
		-I/usr/include/vorbis \
		-I/usr/include/ogg \
		-D_GNU_SOURCE=1 \
		-D_REENTRANT \

#CFLAGS+=-D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/SDL -lSDL

all:
	$(MAKE) pcversion
	$(MAKE) cleanobjs


pcversion: $(SOURCES) $(EXECUTABLEPC)


$(EXECUTABLEPC): $(OBJECTS)
	$(CC) $(LDFLAGS) $(PCLDFLAGS) $(OBJECTS) $(LIBS) -o $@


.c.o:
	$(CC) $(CFLAGS) $< -o $@

cleanobjs:
	rm -f $(OBJECTS)

clean:
	-rm -f tworld *~ *.o *.bak oshw-sdl/*.o oshw-sdl/*.bak oshw-sdl/*~
