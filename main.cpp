#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

#include <SDL2/SDL.h>

#include "image.h"
#include "cpu_chip8.h"
#include "sdl_viewer.h"

// TODO: Emulate at 480Hz
// At 480Hz, counters decrement once every 8 ops.
// Run 480 cycles, then sleep up to a second.

/*
while (!quit) {
  time starttime = now();
  for (int i = 0; i < 480; i++) {
    bool waitkey = RunCycle();
    if (waitkey) break;
    if (cpu.frameChanged()) {
      cpu.GetFrame()->CopyToRGB24(rgb24, 255, 20, 20);
      viewer.SetFrameRGB24(rgb24, emulated_width, emulated_height);
      auto events = viewer.Update();
    }
  }
  // Update at least once
  auto events = viewer.Update();
  cpu.SetKeysFromEvents(events);
  if events.quit { quit = true;}

  time stoptime = now();
  dur to_sec = dur::Seconds(1) - (stoptime - starttime)
  if (to_sec <= 0) { WARN("TOO SLOW");}
  sleep_for(to_sec);
}
*/
using Clock = std::chrono::steady_clock;

void Run() {
  CpuChip8 cpu;
  cpu.Initialize();
  cpu.LoadROM("/Users/river/code/chip8/roms/TETRIS");

  int emulated_width = 64;
  int emulated_height = 32;

  SDLViewer viewer("c8-emu", emulated_width * 8, emulated_height * 8);
  bool quit = false;

  uint8_t* rgb24 = static_cast<uint8_t*>(std::calloc(
      emulated_width * emulated_height * 3, sizeof(uint8_t)));
  while (!quit) {
    auto start_time = Clock::now();
    for (int i = 0; i < CpuChip8::kCycleSpeedHz; ++i) {
      cpu.RunCycle();
    }
    auto stop_time = Clock::now();
    cpu.GetFrame()->CopyToRGB24(rgb24, 255, 20, 20);
    viewer.SetFrameRGB24(rgb24, emulated_width, emulated_height);
    auto events = viewer.Update();

    for (const auto& event : events) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }
    std::chrono::duration<double> diff = stop_time - start_time;
    std::cout << "\nTook " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " ms";
    std::chrono::duration<double> to_second = std::chrono::seconds(1) - diff;
    if (to_second <= Clock::duration::zero()) {
      throw std::runtime_error("Emulation speed unable to maintain 480Hz. Too many draws?");
    }
    std::cout << "\t sleeping for " << std::chrono::duration_cast<std::chrono::milliseconds>(to_second).count() << " ms" << std::endl;
    std::this_thread::sleep_for(to_second);
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
