CXX=g++
RM=rm -f
SDL2CFLAGS=-I/usr/local/include/SDL2 -D_THREAD_SAFE
CXXFLAGS=-O2 -c --std=c++14 -Wall $(SDL2CFLAGS)

# Load dynamic libs here
LDFLAGS=-L/usr/local/lib -lSDL2

chip8: main.o image.o cpu_chip8.o sdl_viewer.o sdl_timer.o
	$(CXX) $(LDFLAGS) -o chip8 main.o image.o cpu_chip8.o sdl_viewer.o sdl_timer.o

main.o: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp

image.o: image.cpp image.h
	$(CXX) $(CXXFLAGS) image.cpp

cpu_chip8.o: cpu_chip8.cpp cpu_chip8.h
	$(CXX) $(CXXFLAGS) cpu_chip8.cpp

sdl_viewer.o: sdl_viewer.cpp sdl_viewer.h
	$(CXX) $(CXXFLAGS) sdl_viewer.cpp

sdl_timer.o: sdl_timer.cpp sdl_timer.h
	$(CXX) $(CXXFLAGS) sdl_timer.cpp

clean:
	$(RM) chip8 *.o