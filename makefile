CC = gcc
CFLAGS = -Wall -Wextra -O3 -Isrc -I ./third-party
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32 -lfftw3f-3 

# options you might find useful

# CFLAGS += $(shell pkg-config --cflags sdl2 fftw3f)
# LDFLAGS += $(shell pkg-config --libs sdl2 fftw3f)

# ifeq ($(OS),Windows_NT)
#     MKDIR_BUILD = if not exist build mkdir build
#     MKDIR_BIN = if not exist bin mkdir bin
#     RM_CMD = rm -rf build bin
# else
    MKDIR_BUILD = mkdir -p build
    MKDIR_BIN = mkdir -p bin
    RM_CMD = rm -rf build bin
# endif

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
SRC = $(call rwildcard,src/,*.c)
SRC += $(call rwildcard,third-party/,*.c)

VPATH = $(sort $(dir $(SRC)))

OBJ = $(patsubst %.c, build/%.o, $(notdir $(SRC)))

TARGET = bin/engine

.PHONY: docs clean-docs

make: $(TARGET)

docs:
	doxygen Doxyfile	

clean-docs:
	rm -rf docs

$(TARGET): $(OBJ)
	@$(MKDIR_BIN)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: %.c
	@$(MKDIR_BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM_CMD)	

