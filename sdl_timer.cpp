#include "sdl_timer.h"

#include <SDL2/SDL.h>

#include "common.h"

SDLTimer::SDLTimer() : start_ticks_(0) {}

void SDLTimer::Start() {
  start_ticks_ = SDL_GetTicks();
}

uint32_t SDLTimer::Ms() {
  return SDL_GetTicks() - start_ticks_;
}