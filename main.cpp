#include <iostream>
#include <vector>
#include <string>

#include <SDL2/SDL.h>

#include "image.h"
#include "cpu_chip8.h"
#include "sdl_viewer.h"

// TODO: Emulate at 480Hz
// At 480Hz, counters decrement once every 8 ops.
// Run 480 cycles, then sleep up to a second.

void Run() {
  CpuChip8 cpu;
  cpu.Initialize();
  cpu.LoadROM("/Users/river/code/chip8/roms/VERS");

  int emulated_width = 64;
  int emulated_height = 32;

  SDLViewer viewer("c8-emu", emulated_width * 8, emulated_height * 8);
  bool quit = false;

  uint8_t* rgb24 = static_cast<uint8_t*>(std::calloc(
      emulated_width * emulated_height * 3, sizeof(uint8_t)));
  while (!quit) {
    cpu.RunCycle();
    cpu.GetFrame()->CopyToRGB24(rgb24, 255, 20, 20);
    viewer.SetFrameRGB24(rgb24, emulated_width, emulated_height);
    auto events = viewer.Update();

    for (const auto& event : events) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }
  }

  free(rgb24);
}

int main() {
  try {
    Run();
    std::cout << "Exit main() success";
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what();
    return 1;
  }
}
