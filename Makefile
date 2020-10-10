PRGNAME     = handy
CC			= gcc
CXX			= g++

PORT = GCW0
SOUND_OUTPUT = alsa
PROFILE = 0

SRCDIR		= ./src/ ./src/gui/ ./src/handy-libretro src/ports ./src/ports/input/sdl src/sdlemu ./src/unzip
VPATH		= $(SRCDIR)
SRC_C		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
SRC_CP		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.cpp))
OBJ_C		= $(notdir $(patsubst %.c, %.o, $(SRC_C)))
OBJ_CP		= $(notdir $(patsubst %.cpp, %.o, $(SRC_CP)))
OBJS		= $(OBJ_C) $(OBJ_CP)

CFLAGS		= -O0 -g -fpermissive
CFLAGS		+= -DWANT_CRC32 -DANSI_GCC -DSDL_PATCH -D$(PORT)
CFLAGS		+= -I./src -I./src/handy-libretro -I./src/sdlemu -Isrc/ports -Isrc/ports/sound -Isrc/ports/sound -Isrc/ports/input/sdl

CFLAGS 		+= -D$(PORT)
SRCDIR		+= ./src/ports/graphics/$(PORT)
SRCDIR		+= ./src/ports/sound/$(SOUND_OUTPUT)

ifeq ($(PROFILE), YES)
CFLAGS 		+= -fprofile-generate=/home/retrofw/profile
else ifeq ($(PROFILE), APPLY)
CFLAGS		+= -fprofile-use -fbranch-probabilities
endif

CXXFLAGS	= $(CFLAGS)
LDFLAGS     = -nodefaultlibs -lc -lstdc++ -lgcc -lgcc_s -lm -lSDL -lz -no-pie -Wl,--as-needed -Wl,--gc-sections -flto

ifeq ($(SOUND_OUTPUT), portaudio)
LDFLAGS		+= -lportaudio
endif
ifeq ($(SOUND_OUTPUT), libao)
LDFLAGS		+= -lao
endif
ifeq ($(SOUND_OUTPUT), alsa)
LDFLAGS		+= -lasound
endif

# Rules to make executable
$(PRGNAME): $(OBJS)  
	$(CC) $(CFLAGS) -o $(PRGNAME) $^ $(LDFLAGS)

$(OBJ_C) : %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_CP) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(PRGNAME) *.o
