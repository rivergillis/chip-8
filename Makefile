CXX=g++
RM=rm -f
CXXFLAGS=-c --std=c++11 -Wall -D DEBUG

# Load dynamic libs here
LDFLAGS=

chip8: main.o image.o cpu_chip8.o
	$(CXX) $(LDFLAGS) -o chip8 main.o image.o cpu_chip8.o

main.o: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp

image.o: image.cpp image.h
	$(CXX) $(CXXFLAGS) image.cpp

cpu_chip8.o: cpu_chip8.cpp cpu_chip8.h
	$(CXX) $(CXXFLAGS) cpu_chip8.cpp

clean:
	$(RM) chip8 *.o