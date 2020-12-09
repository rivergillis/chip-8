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

using Clock = std::chrono::steady_clock;

void Run() {
  int emulated_width = 64;
  int emulated_height = 32;

  SDLViewer viewer("c8-emu", emulated_width, emulated_height, 8);
  std::mutex frame_mutex; // protects rgb24
  uint8_t* rgb24 = static_cast<uint8_t*>(std::calloc(
      emulated_width * emulated_height * 3, sizeof(uint8_t)));
  viewer.SetFrameRGB24(rgb24, emulated_height);

  std::mutex events_mutex; // protects events
  std::vector<SDL_Event> events;

  CpuChip8::Options cpu_options;
  cpu_options.rom_filename = "/Users/river/code/chip8/roms/VERS";
  // cpu_options.rom_filename = "C:/Users/jrive/code/chip-8/roms/TETRIS";
  cpu_options.produce_frame_callback =
    [emulated_height, rgb24, &frame_mutex, &viewer](Image* cpu_img) {
      const std::lock_guard<std::mutex> frame_lock(frame_mutex);
      cpu_img->CopyToRGB24(rgb24, 255, 20, 20);
      viewer.SetFrameRGB24(rgb24, emulated_height);
  }; 
  cpu_options.set_keypad_state_callback = [&events, &events_mutex](uint8_t* cpu_keypad) {
    const std::lock_guard<std::mutex> events_lock(events_mutex);
    for (const auto& e : events) {
      if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
        if (e.key.keysym.sym == SDLK_1) {                        
          cpu_keypad[0] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_2) {                        
          cpu_keypad[1] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_3) {                        
          cpu_keypad[2] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_4) {                        
          cpu_keypad[3] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_q) {                        
          cpu_keypad[4] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_w) {                        
          cpu_keypad[5] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_e) {                        
          cpu_keypad[6] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_r) {                        
          cpu_keypad[7] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_a) {                        
          cpu_keypad[8] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_s) {                        
          cpu_keypad[9] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_d) {                        
          cpu_keypad[10] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_f) {                        
          cpu_keypad[11] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_z) {                        
          cpu_keypad[12] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_x) {                        
          cpu_keypad[13] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_c) {                        
          cpu_keypad[14] = e.type == SDL_KEYDOWN;
        } else if (e.key.keysym.sym == SDLK_v) {                        
          cpu_keypad[15] = e.type == SDL_KEYDOWN;
        }
      }
    }
  };
  CpuChip8 cpu(cpu_options);

  cpu.Start();
  bool quit = false;
  while (!quit) {
    auto new_events = viewer.Update();
    for (const auto& e : new_events) {
      if (e.type == SDL_QUIT) {
        quit = true;
        continue;
      }
    }

    {
      const std::lock_guard<std::mutex> events_lock(events_mutex);
      events.insert(events.end(), new_events.begin(), new_events.end());
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
