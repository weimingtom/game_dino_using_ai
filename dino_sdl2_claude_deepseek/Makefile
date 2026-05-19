# Build the Dino Run game (requires SDL2 + SDL2_ttf)
#
# On MSYS2 / MinGW-w64:
#   pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
#   make
#
# On Linux:
#   sudo apt install libsdl2-dev libsdl2-ttf-dev
#   make
#
# On macOS:
#   brew install sdl2 sdl2_ttf
#   make

CC       = gcc
CFLAGS  += -Wall -Wextra -O2 -std=c99
LDFLAGS += -lSDL2 -lSDL2_ttf -lm

TARGET   = dino_game
SRC      = dino_game.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.exe
