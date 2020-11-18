#include "sdl_viewer.h"

#include <SDL2/SDL.h>

#include "common.h"

SDLViewer::SDLViewer(const std::string& title, int width, int height) : 
      title_(title), window_width_(width), window_height_(height) {
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    throw std::runtime_error(SDL_GetError());
  }
  window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
  if (!window_) {
    throw std::runtime_error(SDL_GetError());
  }
  window_surf_ = SDL_GetWindowSurface(window_);
}

SDLViewer::~SDLViewer() {
  if (frame_surf_) SDL_FreeSurface(frame_surf_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}


std::vector<SDL_Event> SDLViewer::Update() {
  std::vector<SDL_Event> events;
  if (!frame_surf_) {
    throw std::runtime_error("Need to set the frame before calling Update().");
  }

  SDL_Event e;
  while (SDL_PollEvent(&e)) { events.push_back(e); }
  SDL_Rect strech = {0, 0, window_width_, window_height_};
  SDL_BlitScaled(frame_surf_, nullptr, window_surf_, &strech);
  SDL_UpdateWindowSurface(window_);

  return events;
}

void SDLViewer::SetFrameRGB24(uint8_t* rgb24, int width, int height) {
  if (frame_surf_) SDL_FreeSurface(frame_surf_);
  frame_surf_ = SDL_CreateRGBSurfaceWithFormatFrom(rgb24, width, height,
      24, width * 3, SDL_PIXELFORMAT_RGB24);
  if (!frame_surf_) {
    throw std::runtime_error(SDL_GetError());
  }
}