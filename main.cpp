#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

#include <SDL2/SDL.h>

#include "image.h"
#include "cpu_chip8.h"
#include "sdl_viewer.h"

// TODO: Fix timers
// TODO: CPU on worker thread. Ctor takes callback that executes
// when a frame update happens. Main thread makes that callback copy to rgb24
// and update the viewer frame.
//  Inputs? ...

using Clock = std::chrono::steady_clock;
constexpr int kFrameUpdatesPerSecond = 60;

void Run() {
  CpuChip8 cpu;
  cpu.Initialize();
  cpu.LoadROM("/Users/river/code/chip8/roms/PONG");

  int emulated_width = 64;
  int emulated_height = 32;

  SDLViewer viewer("c8-emu", emulated_width, emulated_height, 8);
  uint8_t* rgb24 = static_cast<uint8_t*>(std::calloc(
      emulated_width * emulated_height * 3, sizeof(uint8_t)));
  viewer.SetFrameRGB24(rgb24, emulated_height);
  bool quit = false;
  while (!quit) {
    std::vector<SDL_Event> events;
    auto start_time = Clock::now();
    bool updated = false;
    int updates = 0;
    for (int i = 0; i < CpuChip8::kCycleSpeedHz; ++i) {
      cpu.RunCycle();
      if (cpu.FrameChanged() && updates < kFrameUpdatesPerSecond) {
        cpu.GetFrame()->CopyToRGB24(rgb24, 255, 20, 20);
        viewer.SetFrameRGB24(rgb24, emulated_height);
        auto new_events = viewer.Update();
        events.insert(events.end(), new_events.begin(), new_events.end());
        updates++;
      }
    }
    if (updates == 0) {
      auto new_events = viewer.Update();
      events.insert(events.end(), new_events.begin(), new_events.end());
    }
    auto stop_time = Clock::now();

    for (const auto& event : events) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }
    std::chrono::duration<double> diff = stop_time - start_time;
    std::cout << "\nTook " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " ms";
    std::chrono::duration<double> to_second = std::chrono::seconds(1) - diff;
    if (to_second > Clock::duration::zero()) {
      std::cout << "\t sleeping for " << std::chrono::duration_cast<std::chrono::milliseconds>(to_second).count() << " ms";
      std::this_thread::sleep_for(to_second);
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
