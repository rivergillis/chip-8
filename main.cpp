#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include <thread>

#include <SDL2/SDL.h>

#include "image.h"
#include "cpu_chip8.h"
#include "sdl_viewer.h"

// TODO: Fix timers
// TODO: Now we can display every frame, but the cpu sleep is making it look bad...

using Clock = std::chrono::steady_clock;

void Run() {
  int emulated_width = 64;
  int emulated_height = 32;

  SDLViewer viewer("c8-emu", emulated_width, emulated_height, 8);
  std::mutex frame_mutex; // protects rgb24
  uint8_t* rgb24 = static_cast<uint8_t*>(std::calloc(
      emulated_width * emulated_height * 3, sizeof(uint8_t)));
  viewer.SetFrameRGB24(rgb24, emulated_height);

  CpuChip8::Options cpu_options;
  //cpu_options.rom_filename = "/Users/river/code/chip8/roms/PONG2";
  cpu_options.rom_filename = "C:/Users/jrive/code/chip-8/roms/TETRIS";
  cpu_options.produce_frame_callback =
    [emulated_height, rgb24, &frame_mutex, &viewer](Image* cpu_img) {
      const std::lock_guard<std::mutex> frame_lock(frame_mutex);
      cpu_img->CopyToRGB24(rgb24, 255, 20, 20);
      viewer.SetFrameRGB24(rgb24, emulated_height);
  }; 
  cpu_options.set_keypad_state_callback = [](uint8_t* cpu_keypad) {}; // todo;
  CpuChip8 cpu(cpu_options);

  cpu.Start();
  bool quit = false;
  while (!quit) {
    std::vector<SDL_Event> events;
    auto new_events = viewer.Update();
    events.insert(events.end(), new_events.begin(), new_events.end());

    for (const auto& event : events) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }
    // Give the CPU thread time to run.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  cpu.Stop();

  free(rgb24);
}

int main(int argc, char* args[]) {
  try {
    Run();
    std::cout << "Exit main() success";
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what();
    return 1;
  }
}
