CC := gcc
SRCDIR := src
RESDIR := res
CFLAGS_COMMON := -Isrc -O2 -Wall -Werror -Wno-unused-function -std=c99 -g
CFLAGS_COMMON += -fomit-frame-pointer -fomit-frame-pointer
LDFLAGS_COMMON := 
OBJDIR := obj

TARGET_EXEC := cursor

CFLAGS_CUSTOM += -DWANT_FAST_START

SOURCES := $(shell find $(SRCDIR)/ -type f -name '*.c')
RESOURCES := $(shell find $(RESDIR)/ -type f -name '*.rc')
OBJECTS := $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))

# windows-specific settings
ifdef SYSTEMROOT
	CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_CUSTOM) `sdl2-config --cflags` -mconsole 
	LDFLAGS := $(LDFLAGS_COMMON) `sdl2-config --static-libs` -lSDL2_image -L. -lmingw32 -lm
	OBJECTS += $(RESOURCES:.rc=.res)
else	
	CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_CUSTOM) `sdl2-config --cflags` -fsanitize=address -fsanitize=undefined
	LDFLAGS := $(LDFLAGS_COMMON) `sdl2-config --libs` -lSDL2_image -lm -fsanitize=address
endif


.PHONY: all clean

all: $(TARGET_EXEC)

clean:
	$(RM) $(OBJECTS) $(TARGET_EXEC)
	$(RM) -rf $(OBJDIR)

masks:
	@bash -c 'printf "\t\e[92m[ MOG ]\e[0m Generating object masks...\n"'
	mkdir -p res/gfx/obj/mask
	@mogrify -path res/gfx/obj/mask -alpha extract -transparent "rgb(0,0,0)" res/gfx/obj/*.png

test: $(TARGET_EXEC)
	$(TARGET_EXEC) 1280 720

$(TARGET_EXEC): $(OBJECTS)
	@bash -c 'printf "\t\e[94m[ LNK ]\e[0m $(OBJECTS)\n"'
	@$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)/$(<D)
	@bash -c 'printf "\t\e[96m[  C  ]\e[0m $<\n"'
	@$(CC) -c $(CFLAGS) $< -o $@

%.res: %.rc
	@bash -c 'printf "\t\e[95m[ RES ]\e[0m $<\n"'
	@windres $< -O coff -o $@
