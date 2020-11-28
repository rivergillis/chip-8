#include "sdl_viewer.h"

#include <mutex>
#include <SDL2/SDL.h>

#include "common.h"

SDLViewer::SDLViewer(const std::string& title, int width, int height, int window_scale) : 
      title_(title) {
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    throw std::runtime_error(SDL_GetError());
  }
  window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, width * window_scale, height * window_scale, SDL_WINDOW_SHOWN);
  if (!window_) {
    throw std::runtime_error(SDL_GetError());
  }
  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer_) {
    throw std::runtime_error(SDL_GetError());
  }
  SDL_SetRenderDrawColor(renderer_, 0xFF, 0xFF, 0xFF, 0xFF);

  window_tex_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24,
    SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!window_tex_) {
    throw std::runtime_error(SDL_GetError());
  }

  timer_.Start();
}

SDLViewer::~SDLViewer() {
  SDL_DestroyTexture(window_tex_);
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}


std::vector<SDL_Event> SDLViewer::Update() {
  const std::lock_guard<std::mutex> lock(mu_);
  std::vector<SDL_Event> events;
  if (!window_tex_) {
    throw std::runtime_error("Need to set the frame before calling Update().");
  }

  SDL_Event e;
  while (SDL_PollEvent(&e)) { events.push_back(e); }

  SDL_RenderCopy(renderer_, window_tex_, NULL, NULL );
  SDL_RenderPresent(renderer_);

  ++num_updates_;

  // Compute fps and set window title.
  float avg_fps = num_updates_ / (timer_.Ms() / 1000.0f);
  SDL_SetWindowTitle(window_, 
    (title_ + " - " + std::to_string(static_cast<int>(avg_fps)) + "fps").c_str());
  if (timer_.Ms() >= 1'000) {
    num_updates_ = 0;
    timer_.Start();
  }

  return events;
}

void SDLViewer::SetFrameRGB24(uint8_t* rgb24, int height) {
  const std::lock_guard<std::mutex> lock(mu_);
  void* pixeldata;
  int pitch;
  // Lock the texture and upload the image to the GPU.
  SDL_LockTexture(window_tex_, nullptr, &pixeldata, &pitch);
  std::memcpy(pixeldata, rgb24, pitch * height);
  SDL_UnlockTexture(window_tex_);
}