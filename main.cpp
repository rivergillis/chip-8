#include <iostream>
#include <vector>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "image.h"
#include "cpu_chip8.h"

// TODO: Emulate at 60hz. (60 cycles per second).
void Run() {
  CpuChip8 cpu;
  cpu.Initialize();
  cpu.LoadROM("/Users/river/code/chip8/roms/PONG");
  for (int i = 0; i < 500; ++i) {
    cpu.RunCycle();
  }
  std::cout << "Exit main() success";

      if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL init failed." << std::endl;
        return;
    }

    std::cout << "SDL Init succeeded." << std::endl;

    SDL_Quit();
}

int main() {
  try {
    Run();
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what();
    return 1;
  }
}
