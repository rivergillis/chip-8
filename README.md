0. brew install sdl2 sdl2_image sdl2_mixer sdl2_net sdl2_ttf
1. Makefile: set SDL2CFLAGS to output of sdl2-config --cflags
2. Makefile: set LDFLAGS to output of sdl2-config --libs
3. make chip8
4. ./chip8
